#include "resindex.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/refcounted.h"
#include "../core/vcall.h"
#include "handles.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

Handle *HandleAt(u8 *object, u32 offset) { return reinterpret_cast<Handle *>(object + offset); }

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
