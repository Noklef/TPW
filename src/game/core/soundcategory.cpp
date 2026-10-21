#include "soundcategory.h"

#include <cstdint>
#include <cstring>

#include "../files/handles.h"
#include "../files/file.h"
#include "dequeiter.h"
#include "misc.h"
#include "gameheap.h"
#include "methods.h"
#include "ptrlist.h"
#include "seqs.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T Get(const void *p, u32 offset) {
    T v;
    std::memcpy(&v, static_cast<const u8 *>(p) + offset, sizeof v);
    return v;
}
template <typename T> void Put(void *p, u32 offset, T v) { std::memcpy(static_cast<u8 *>(p) + offset, &v, sizeof v); }
u8 *Ptr(u32 address) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(address)); }
u32 Addr(const void *p) { return static_cast<u32>(reinterpret_cast<uintptr_t>(p)); }

// The element n dwords on from a deque iterator {first, end, position, map slot} (the originals' inline form of DequeIter_Advance).
u8 *DequeAt(const u8 *it, u32 n) {
    u32 at = Get<u32>(it, 8) + n * 4;
    s32 offset = static_cast<s32>(at - Get<u32>(it, 0)) >> 2;
    const s32 blocks = offset >= 0 ? static_cast<s32>(static_cast<u32>(offset) >> 10) : -static_cast<s32>((0x3ffu - static_cast<u32>(offset)) >> 10);
    if (blocks) {
        offset -= blocks << 10;
        at = Get<u32>(Ptr(Get<u32>(it, 0xc)), static_cast<u32>(blocks) * 4) + static_cast<u32>(offset) * 4;
    }
    return Ptr(at);
}

// A copy of the iterator at it, moved n on (DequeIter_Advance); its position.
u8 *Advanced(const u8 *it, s32 n) {
    u8 copy[0x10];
    std::memcpy(copy, it, sizeof copy);
    DequeIter_Advance(copy, nullptr, n);
    return Ptr(Get<u32>(copy, 8));
}

// The deques' inline push_back (0x6c1940 when the end's block is full or the deque empty).
void PushBack(u8 *deque, u32 value) {
    if (!Get<u32>(deque, 0x2c) || Get<u32>(deque, 0x1c) == Get<u32>(deque, 0x18)) Deque32_GrowBack(deque);
    const u32 cur = Get<u32>(deque, 0x1c);
    Put<u32>(deque, 0x1c, cur + 4);
    if (cur) Put<u32>(Ptr(cur), 0, value);
    Put<u32>(deque, 0x2c, Get<u32>(deque, 0x2c) + 1);
}

const StringView *View(const String *s) { return reinterpret_cast<const StringView *>(s); }
void *const *Vtable(u32 address) { return reinterpret_cast<void *const *>(static_cast<uintptr_t>(address)); }

}  // namespace

extern "C" {

u32 __cdecl SfxVariation_SampleCount(const u8 *variation) { return Get<u32>(variation, 0) & 0xffff; }

void __fastcall SfxMap_MarkSimpleSound(u8 *, void *, u8 *sound) {
    const u16 flags = Get<u16>(sound, 0x10);
    if (!(flags & 4) || !(flags & 2) || Get<u32>(sound, 4) != 1 || SfxVariation_SampleCount(Ptr(Get<u32>(sound, 8))) != 1) return;
    const u8 *const variation = Ptr(Get<u32>(sound, 8));
    if (Get<u32>(variation, 0x26) || Get<u16>(variation, 0x10) || Get<u16>(variation, 0x12)) return;
    Put<u16>(sound, 0x10, static_cast<u16>((Get<u16>(sound, 0x10) & 0xfffb) | 8));
}

u8 *__fastcall SfxMap_FixVariation(u8 *category, void *, u8 *data, u8 *variation) {
    Put<u32>(variation, 8, Addr(data));
    u8 *const links = data + SfxVariation_SampleCount(variation) * 0x10;
    Put<u32>(variation, 0x26, Addr(links));
    u8 *const end = links + Get<u32>(variation, 4) * 8;
    u8 *bank = Ptr(Get<u32>(variation, 8)) + 0xc;
    for (u32 i = 0; i < SfxVariation_SampleCount(variation); ++i, bank += 0x10) {
        const u32 number = Get<u16>(bank, 0);
        if (!number) continue;
        Put<u16>(bank, 0, Get<u16>(Advanced(category + 0x9c, static_cast<s32>(number - 1)), 0));
        const u32 index = Get<u32>(category, 0xc8);
        const u8 *const record = Ptr(Get<u32>(Advanced(category + 0x3c, static_cast<s32>(index)), 0));
        const u32 wanted = Get<u16>(bank, 0);
        u32 node = Get<u32>(record, 0x28);
        while (node && Get<u32>(Ptr(node), 0) != wanted) node = Get<u32>(Ptr(node), 4);
        if (node) continue;
        u8 *const owner = Ptr(Get<u32>(DequeAt(category + 0x3c, Get<u32>(category, 0xc8)), 0));
        PtrList_Append(reinterpret_cast<PtrList *>(owner + 0x24), nullptr, reinterpret_cast<void *>(static_cast<uintptr_t>(Get<u16>(bank, 0))));
    }
    return end;
}

u8 *__fastcall SfxMap_FixSound(u8 *category, void *, u8 *data, u8 *sound) {
    const u32 count = Get<u32>(sound, 4);
    Put<u32>(sound, 8, Addr(data));
    u8 *at = data + count * 0x2a;
    for (u32 i = 0; i < Get<u32>(sound, 4); ++i) at = SfxMap_FixVariation(category, nullptr, at, Ptr(Get<u32>(sound, 8)) + 0x2a * i);
    if (!Get<u32>(sound, 4)) return at;
    u8 *weight = Ptr(Get<u32>(sound, 8)) + 0x1e;
    u32 previous = 0;
    for (u32 i = 0; i < Get<u32>(sound, 4); ++i, weight += 0x2a) {
        if (category[0xcc]) {
            const u32 w = Get<u32>(weight, 0);
            Put<u32>(weight, 0, w - previous);
            previous = w;
        }
        u8 *link = Ptr(Get<u32>(weight, 8));
        for (u32 j = 0; j < Get<u32>(weight - 0x1a, 0); ++j, link += 8) Put<u32>(link, 0, Get<u32>(sound, 8) + Get<u32>(link, 0) * 0x2a - 0x2a);
    }
    return at;
}

u8 *__fastcall SfxMap_FixCategory(u8 *category, void *, u8 *data, u8 *record) {
    const u32 count = Get<u32>(record, 0);
    Put<u32>(record, 4, Addr(data));
    u8 *at = data + count * 0x14;
    if (!count) return at;
    for (u32 i = 0; i < Get<u32>(record, 0); ++i) {
        at = SfxMap_FixSound(category, nullptr, at, Ptr(Get<u32>(record, 4)) + 0x14 * i);
        SfxMap_MarkSimpleSound(category, nullptr, Ptr(Get<u32>(record, 4)) + 0x14 * i);
    }
    return at;
}

u8 *__fastcall SfxMap_Fixup(u8 *category, void *, u8 *data, u8 **out, u32 count) {
    *out = data;
    u8 *record = *out;
    u8 *at = data + count * 0x18;
    for (; count; --count, record += 0x18) at = SfxMap_FixCategory(category, nullptr, at, record);
    return at;
}

u32 __fastcall SoundCategory_FreeCategorySlot(const u8 *category) {
    const u32 count = Get<u32>(category, 0x64);
    u32 i = 0;
    for (; i < count; ++i)
        if (!Get<u32>(DequeAt(category + 0x3c, i), 0)) break;
    return i;
}

u32 __fastcall SoundCategory_FreeBankSlot(const u8 *category) {
    u32 cur = Get<u32>(category, 0x74);
    u32 last = Get<u32>(category, 0x70);
    u32 node = Get<u32>(category, 0x78);
    u32 n = 0;
    while (cur != Get<u32>(category, 0x84) && Get<u32>(Ptr(cur), 0)) {
        ++n;
        cur += 4;
        if (cur == last) {
            node += 4;
            cur = Get<u32>(Ptr(node), 0);
            last = cur + 0x1000;
        }
    }
    return n;
}

void __fastcall SoundCategory_ReadBankEntries(u8 *, void *, StreamBase *stream, u32 *table) {
    vcall<s32>(stream, 0x18);  // (its result unused)
    const u32 size = static_cast<u32>(vcall<s32>(stream, 0x14));
    const u32 bytes = size - static_cast<u32>(vcall<s32>(stream, 0x18));
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(bytes));
    vcall<s32>(stream, 8, static_cast<void *>(block), bytes);
    table[0] = Addr(block);
    const u32 count = table[1];
    u8 *path = block + count * 11;
    u8 *entry = block;
    for (u32 i = 0; i < count; ++i, entry += 11) {
        const u32 length = Get<u32>(path, 0);
        path += 4;
        Put<u32>(entry, 5, Addr(path));
        path += length;
        Put<u32>(entry, 0, 0);
        entry[4] = 0;
        Path_AfterLastBackslash(reinterpret_cast<const char **>(entry + 5), length);
    }
}

void __fastcall SoundCategory_OpenBankFolder(u8 *, void *, BankFolder *found, const String *folder) {
    if (found->found) return;
    String path;
    String_CtorCopy(&path, nullptr, View(folder));
    String_AssignStr(&path, nullptr, folder->data);
    const u32 length = path.length;
    if (String_ReverseFind(&path, nullptr, "\\", 0xffffffffu) != static_cast<s32>(length - 1)) String_AppendStr(&path, nullptr, "\\");
    const s32 last = String_ReverseFind(&path, nullptr, "\\", 0xffffffffu);
    u32 wad = 0;
    if (last != -1) {
        StringView left;
        String_Assign(&path, nullptr, String_Left(&path, nullptr, &left, static_cast<u32>(last)));
        String_AppendStr(&path, nullptr, ".wad");
        wad = File_Exists(path.data);
    }
    Handle handle;
    s32 status;
    Handle_Ctor(&handle);
    if (wad) {
        handle.vtable = Vtable(0x6fda18);
        if (*Handle_OpenE0(&handle, nullptr, &status, Addr(path.data), 0) >= 0) {
            Handle_Assign(&found->handle, nullptr, &handle);
            found->found = 1;
        }
    } else {
        handle.vtable = Vtable(0x6fd9e8);
        if (*Handle_Open28(&handle, nullptr, &status, Addr(folder->data)) >= 0) {
            Handle_Assign(&found->handle, nullptr, &handle);
            found->found = 1;
        }
    }
    Handle_Dtor(&handle);
    String_Dtor(&path);
}

s32 *__fastcall SoundCategory_SetFolder(u8 *category, void *, s32 *result, const char *folder) {
    Handle *const held = reinterpret_cast<Handle *>(category + 0x30);
    vcall<void>(held, 0);
    BankFolder found;
    found.found = 0;
    Handle_Ctor(&found.handle);
    String name;
    String_CtorStr(&name, nullptr, folder);
    SoundCategory_OpenBankFolder(category, nullptr, &found, &name);
    String_Dtor(&name);
    if (found.found) {
        Handle_Assign(held, nullptr, &found.handle);
        *result = 0;
    } else {
        *result = -1;
    }
    Handle_Dtor(&found.handle);
    return result;
}

bool __fastcall SoundCategory_FindBank(u8 *category, void *, u32 *index, const char *path, const String *folder, BankFolder *found) {
    String held, wanted;
    String_Ctor(&held);
    String_Ctor(&wanted);
    found->found = 0;
    for (u32 i = 0; i < Get<u32>(category, 0x94); ++i) {
        if (!Get<u32>(Advanced(category + 0x6c, static_cast<s32>(i)), 0)) continue;
        String empty;
        String_CtorStr(&empty, nullptr, "");
        u8 *const bank = Ptr(Get<u32>(Advanced(category + 0x6c, static_cast<s32>(i)), 0));
        s32 status;
        vcall<s32 *>(bank + 0x24, 0x10, &status, &held, &empty);
        String_Dtor(&empty);
        String_AssignWide(&wanted, nullptr, reinterpret_cast<const WString *>(folder));
        if (String_Equals(&held, nullptr, View(&wanted))) {
            found->found = 1;
            Handle_Assign(&found->handle, nullptr, reinterpret_cast<const Handle *>(Ptr(Get<u32>(Advanced(category + 0x6c, static_cast<s32>(i)), 0)) + 0x24));
        }
        const u8 *const named = Ptr(Get<u32>(DequeAt(category + 0x6c, i), 0));
        String_Append(&held, nullptr, reinterpret_cast<const StringView *>(named + 0xc));
        String_AppendStr(&wanted, nullptr, path);
        if (String_Equals(&held, nullptr, View(&wanted))) {
            PushBack(category + 0x98, i);
            *index = i;
            String_Dtor(&wanted);
            String_Dtor(&held);
            return false;
        }
    }
    String_Dtor(&wanted);
    String_Dtor(&held);
    return true;
}

s32 *__fastcall SoundCategory_ReadBankMap(u8 *category, void *, s32 *result, StreamBase *stream, const char *folder) {
    u32 header[5];
    vcall<s32>(stream, 8, static_cast<void *>(header), 0x14);
    const u8 *const guid = Ptr(0x70a938);
    if (header[0] == Get<u32>(guid, 0) && header[1] == Get<u32>(guid, 4) && header[2] == Get<u32>(guid, 8) && header[3] == Get<u32>(guid, 0xc)) {
        u32 table[2];
        vcall<s32>(stream, 8, static_cast<void *>(table), 8);
        SoundCategory_ReadBankEntries(category, nullptr, stream, table);
        for (u32 i = 0, at = 0; i < table[1]; ++i, at += 11) {
            String name;
            String_CtorStr(&name, nullptr, folder);
            if (*Ptr(table[0] + at + 9) & 0x20) {
                String_AssignWide(&name, nullptr, reinterpret_cast<const WString *>(Ptr(Get<u32>(Ptr(0xfb1ffc), 0) + 0x30)));
                String_AppendStr(&name, nullptr, "Global\\");
            }
            BankFolder found;
            found.found = 0;
            Handle_Ctor(&found.handle);
            u32 index;
            if (SoundCategory_FindBank(category, nullptr, &index, reinterpret_cast<const char *>(Ptr(Get<u32>(Ptr(table[0] + at), 5))), &name, &found)) {
                u8 *const bank = static_cast<u8 *>(GameHeap_Alloc(0x2c));
                if (bank) {
                    const u8 *const entry = Ptr(table[0] + at);
                    Class703770_Ctor(bank);
                    Put<u32>(bank, 8, 0);
                    String_CtorStr(reinterpret_cast<String *>(bank + 0xc), nullptr, reinterpret_cast<const char *>(Ptr(Get<u32>(entry, 5))));
                    Put<u32>(bank, 0x20, entry[9]);
                    Handle_Ctor(reinterpret_cast<Handle *>(bank + 0x24));
                    Put<u32>(bank, 0, 0x70a960);
                }
                SoundCategory_OpenBankFolder(category, nullptr, &found, &name);
                if (found.found) {
                    Handle_Assign(reinterpret_cast<Handle *>(Ptr(Addr(bank) + 0x24)), nullptr, &found.handle);
                    const u32 slot = SoundCategory_FreeBankSlot(category);
                    if (slot < Get<u32>(category, 0x94))
                        Put<u32>(Advanced(category + 0x6c, static_cast<s32>(slot)), 0, Addr(bank));
                    else
                        PushBack(category + 0x68, Addr(bank));
                    PushBack(category + 0x98, slot);
                }
            } else {
                vcall<void>(Ptr(Get<u32>(Advanced(category + 0x6c, static_cast<s32>(index)), 0)), 4);
            }
            Handle_Dtor(&found.handle);
            String_Dtor(&name);
        }
        GameHeap_Free(Ptr(table[0]));
    }
    *result = 0;
    return result;
}

s32 *__fastcall SoundCategory_LoadBankMap(u8 *category, void *, s32 *result, const char *name, u32) {
    StreamBase stream;
    StreamBase_Ctor(&stream);
    // (The status is the name argument's own stack slot, which holds the name's address until a call writes it, sic.)
    s32 status = static_cast<s32>(reinterpret_cast<uintptr_t>(name));
    Handle *const folder = reinterpret_cast<Handle *>(category + 0x30);
    if (*vcall<s32 *>(folder, 0xc, &status, &stream, name, 0x80000001u, 0u) >= 0) {
        String path, empty;
        String_Ctor(&path);
        String_CtorStr(&empty, nullptr, "");
        vcall<s32 *>(folder, 0x10, &status, &path, &empty);
        String_Dtor(&empty);
        SoundCategory_ReadBankMap(category, nullptr, &status, &stream, path.data);
        StreamBase_Close(&stream);
        String_Dtor(&path);
    }
    *result = 0;
    StreamBase_Dtor(&stream);
    return result;
}

void __fastcall Bank_Dtor(u8 *bank) {
    Handle_Dtor(reinterpret_cast<Handle *>(bank + 0x24));
    String_Dtor(reinterpret_cast<String *>(bank + 0xc));
    Put<u32>(bank, 0, 0x703770);
}

u8 *__fastcall Bank_DeletingDtor(u8 *bank, void *, u32 flags) {
    Bank_Dtor(bank);
    if (flags & 1) GameHeap_Free(bank);
    return bank;
}

s32 *__fastcall SoundCategory_ReadSfxImage(u8 *category, void *, s32 *result, StreamBase *stream, u32 *index) {
    u32 header[5];
    vcall<s32>(stream, 8, static_cast<void *>(header), 0x14);
    if (!Obj_CheckId(category, nullptr, header)) {
        *result = -1;
        return result;
    }
    const u32 size = static_cast<u32>(vcall<s32>(stream, 0x14));  // the size first, then the position
    const u32 bytes = size - static_cast<u32>(vcall<s32>(stream, 0x18));
    u32 head[2];
    vcall<s32>(stream, 8, static_cast<void *>(head), 8);
    u8 *const data = static_cast<u8 *>(GameHeap_Alloc(bytes));
    vcall<s32>(stream, 8, static_cast<void *>(data), bytes);
    u8 *const record = Ptr(Get<u32>(Advanced(category + 0x3c, static_cast<s32>(Get<u32>(category, 0xc8))), 0));
    SfxMap_Fixup(category, nullptr, data, reinterpret_cast<u8 **>(record + 8), head[1]);
    *index = Get<u32>(category, 0xc8);
    *result = 0;
    return result;
}

s32 *__fastcall SoundCategory_ReadSfxMap(u8 *category, void *, s32 *result, const char *name, u32 *index) {
    StreamBase stream;
    StreamBase_Ctor(&stream);
    // (The status is the name argument's own stack slot, which holds the name's address until a call writes it, sic.)
    s32 status = static_cast<s32>(reinterpret_cast<uintptr_t>(name));
    if (*vcall<s32 *>(category + 0x30, 0xc, &status, &stream, name, 0x80000001u, 0u) < 0) {
        *result = -1;
    } else {
        const bool read = *SoundCategory_ReadSfxImage(category, nullptr, &status, &stream, index) >= 0;
        StreamBase_Close(&stream);
        *result = read ? 0 : -1;
    }
    StreamBase_Dtor(&stream);
    return result;
}

}  // extern "C"
