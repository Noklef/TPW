#include "resindex.h"

#include <cstdint>

#include "../core/gameheap.h"
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
