#include "resindex.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/methods.h"
#include "../core/refcounted.h"
#include "../core/seqs.h"
#include "../core/vcall.h"
#include "handles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

Handle *HandleAt(u8 *object, u32 offset) { return reinterpret_cast<Handle *>(object + offset); }

// The folder's typed getters: the index's record for the key fetched when it is of the kind.
s32 *GetOfKind(u8 *folder, s32 *result, u32 key, u8 **object, u32 kind) {
    u8 *const record = ResIndex_Find(at<u8 *>(folder, 0), nullptr, key);
    if (Record_Word4OrSix(record) == kind) return ResIndex_Fetch(at<u8 *>(folder, 0), nullptr, result, record, object);
    *object = nullptr;
    *result = -1;
    return result;
}

}  // namespace

u8 *__fastcall ResFolder_Ctor(u8 *folder) {
    Handle_Ctor(HandleAt(folder, 4));
    at<u32>(folder, 4) = 0x6fd9e8;
    at<u32>(folder, 0xc) = 0;
    at<u32>(folder, 0) = 0;
    return folder;
}

void __fastcall ResFolder_Dtor(u8 *folder) {
    s32 result = (s32)(uintptr_t)folder;
    ResFolder_Close(folder, nullptr, &result);
    Handle_Dtor(HandleAt(folder, 4));
}

s32 *__fastcall ResFolder_Open(u8 *folder, void *, s32 *result, u32 path) {
    if (at<u32>(folder, 0xc) == 0) {
        s32 opened = (s32)path;
        if (*Handle_Open28(HandleAt(folder, 4), nullptr, &opened, path) >= 0) {
            at<u32>(folder, 0xc) = 1;
            *result = 0;
            return result;
        }
    }
    *result = -1;
    return result;
}

s32 *__fastcall ResFolder_Close(u8 *folder, void *, s32 *result) {
    vcall<void>(HandleAt(folder, 4), 0);
    if (u8 *const index = at<u8 *>(folder, 0)) {
        vcall<void>(index, 0);
        at<u32>(folder, 0) = 0;
    }
    *result = 0;
    return result;
}

s32 *__fastcall ResIndex_Init(u8 *index, void *, s32 *result, u32 path, u8 *records, u32 count) {
    s32 opened = (s32)path;
    if (*Handle_Open28(HandleAt(index, 0x10), nullptr, &opened, path) < 0) {
        *result = -1;
        return result;
    }
    at<u8 *>(index, 8) = records;
    at<u32>(index, 0xc) = (u32)(uintptr_t)records + count * 12 - 12;
    u32 key = at<u32>(records, 0);
    at<u32>(records, 8) = 0;
    for (u8 *record = records + 12; (u32)(uintptr_t)record <= at<u32>(index, 0xc); record += 12) {
        if (at<u32>(record, 0) <= key) {
            GameHeap_Free(at<u8 *>(index, 8));
            at<u32>(index, 8) = 0;
            *result = -1;
            return result;
        }
        key = at<u32>(record, 0);
        at<u32>(record, 8) = 0;
    }
    *result = 0;
    return result;
}

u8 *__fastcall ResIndex_Ctor(u8 *index) {
    Class70a078_Ctor(index);
    Handle_Ctor(HandleAt(index, 0x10));
    at<u32>(index, 0x10) = 0x6fd9e8;
    at<u32>(index, 0) = 0x70a128;
    at<u32>(index, 8) = 0;
    at<u32>(index, 0xc) = 0;
    return index;
}

void __fastcall ResIndex_Dtor(u8 *index) {
    at<u32>(index, 0) = 0x70a128;
    ResIndex_ReleaseAll(index);
    Handle_Dtor(HandleAt(index, 0x10));
    at<u32>(index, 0) = 0x70a078;
}

u8 *__fastcall ResIndex_DeletingDtor(u8 *index, void *, u32 flags) {
    ResIndex_Dtor(index);
    if (flags & 1) GameHeap_Free(index);
    return index;
}

void __fastcall ResIndex_ReleaseEntry(u8 *, void *, u8 *record) {
    if (!record) return;
    if (u8 *const object = at<u8 *>(record, 8)) vcall<void>(object, 0xc);
    at<u32>(record, 8) = 0;
}

void __fastcall ResIndex_ReleaseAll(u8 *index) {
    if (u8 *record = at<u8 *>(index, 8)) {
        for (; (u32)(uintptr_t)record <= at<u32>(index, 0xc); record += 12) ResIndex_ReleaseEntry(index, nullptr, record);
        at<u32>(index, 8) = 0;
    }
    vcall<void>(HandleAt(index, 0x10), 0);
}

s32 *__fastcall ResIndex_GetEntry(u8 *, void *, s32 *result, u8 *record, u8 **object) {
    if (record) {
        if (u8 *const held = at<u8 *>(record, 8)) vcall<void>(held, 4);
        u8 *const held = at<u8 *>(record, 8);
        *object = held;
        if (held) {
            *result = 0;
            return result;
        }
    }
    *result = -1;
    return result;
}

s32 *__fastcall ResIndex_OpenEntry(u8 *index, void *, s32 *result, u32 stream, u8 *record) {
    const u32 name = at<u16>(record, 6) + at<u32>(index, 8);
    s32 opened = (s32)stream;
    *result = *vcall<s32 *>(HandleAt(index, 0x10), 0xc, &opened, stream, name, 0x80000001u, 0u) < 0 ? -1 : 0;
    return result;
}

s32 *__fastcall ResIndex_Load(u8 *index, void *, s32 *result, u8 *record, u8 **object) {
    *object = nullptr;
    if (at<u16>(record, 4) >= 6) {
        *result = -1;
        return result;
    }
    StreamBase stream;
    StreamBase_Ctor(&stream);
    s32 slot = (s32)(uintptr_t)object;
    s32 status = -1;
    if (*ResIndex_OpenEntry(index, nullptr, &slot, (u32)(uintptr_t)&stream, record) >= 0) {
        u8 *const loader = *reinterpret_cast<u8 *const *>((uintptr_t)0x782d20 + at<u16>(record, 4) * 4);
        if (loader && *vcall<s32 *>(loader, 0, &slot, &stream, object, at<u32>(record, 0), index) >= 0) {
            u8 *const loaded = *object;
            at<u8 *>(record, 8) = loaded;
            vcall<void>(loaded, 0x10);
            status = 0;
        }
    }
    *result = status;
    StreamBase_Dtor(&stream);
    return result;
}

s32 *__fastcall ResFolder_LoadIndex(u8 *folder, void *, s32 *result, u32 path) {
    if (u8 *const old = at<u8 *>(folder, 0)) {
        vcall<void>(old, 0);
        at<u32>(folder, 0) = 0;
    }
    Handle handle;
    Handle_Ctor(&handle);
    handle.vtable = reinterpret_cast<void *const *>((uintptr_t)0x6fd9e8);
    s32 status = (s32)path;
    if (*Handle_Open28(&handle, nullptr, &status, path) < 0) {
        *result = -1;
        Handle_Dtor(&handle);
        return result;
    }
    StreamBase stream;
    StreamBase_Ctor(&stream);
    s32 outcome = -1;
    if (*Handle_Vfn3(&handle, nullptr, &status, (u32)(uintptr_t)&stream, 0x782d14, 0x80000001u, 0) >= 0) {
        u32 header[2];
        const s32 got = stream.source ? vcall<s32>(stream.source, 4, header, 8u) : -1;
        if (got == 8 && header[0] == 0x49524642) {   // "BFRI"
            const s32 tell = StreamBase_Tell(&stream);
            const u32 size = (u32)StreamBase_Size(&stream) - (u32)tell;
            const u32 count = header[1];
            if (count == 0) {
                s32 slot = (s32)path;
                ResFolder_Close(folder, nullptr, &slot);
            } else if (u8 *const block = static_cast<u8 *>(GameHeap_Alloc(size + 0x18))) {
                const s32 read = stream.source ? vcall<s32>(stream.source, 4, block + 0x18, size) : -1;
                if ((u32)read != size) {
                    GameHeap_Free(block);
                } else {
                    ResIndex_Ctor(block);
                    s32 slot = (s32)path;
                    if (*ResIndex_Init(block, nullptr, &slot, path, block + 0x18, count) < 0) {
                        GameHeap_Free(block);
                    } else {
                        at<u8 *>(folder, 0) = block;
                        outcome = 0;
                    }
                }
            }
        }
    }
    *result = outcome;
    StreamBase_Dtor(&stream);
    Handle_Dtor(&handle);
    return result;
}

s32 *__fastcall ResFolder_Load(u8 *folder, void *, s32 *result, u32 name) {
    char path[0x200];
    s32 status;
    if (*vcall<s32 *>(HandleAt(folder, 4), 0x14, &status, path, name, 0x200u) < 0) {
        *result = -1;
        return result;
    }
    return ResFolder_LoadIndex(folder, nullptr, result, (u32)(uintptr_t)path);
}

u8 *__fastcall ResIndex_Find(u8 *index, void *, u32 key) { return Table12_Find(index, nullptr, key); }

s32 *__fastcall ResIndex_Fetch(u8 *index, void *, s32 *result, u8 *record, u8 **object) {
    *object = nullptr;
    if (!record) {
        *result = -1;
        return result;
    }
    if (u8 *const held = at<u8 *>(record, 8)) {
        *object = held;
        vcall<void>(at<u8 *>(record, 8), 4);
        *result = 0;
        return result;
    }
    return ResIndex_Load(index, nullptr, result, record, object);
}

s32 *__fastcall ResFolder_GetCodeTable(u8 *folder, void *, s32 *result, u8 **object, u32 key) { return GetOfKind(folder, result, key, object, 0); }

s32 *__fastcall ResFolder_GetRangeTable(u8 *folder, void *, s32 *result, u8 **object, u32 key) { return GetOfKind(folder, result, key, object, 1); }

s32 *__fastcall ResFolder_GetCharset(u8 *folder, void *, s32 *result, u8 **object, u32 key) { return GetOfKind(folder, result, key, object, 3); }

s32 *__fastcall ResFolder_GetFont(u8 *folder, void *, s32 *result, u8 **object, u32 key) { return GetOfKind(folder, result, key, object, 4); }
