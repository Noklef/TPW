#include "soundcategory.h"

#include <cstdint>
#include <cstring>

#include "../files/handles.h"
#include "dequeiter.h"
#include "gameheap.h"
#include "methods.h"
#include "ptrlist.h"
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
