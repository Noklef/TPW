#include "small44.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall IntArray_SetSize(u8 *array, void *, s32 newSize, s32 growBy) {
    if (growBy != -1) at<s32>(array, 0xc) = growBy;
    if (newSize == 0) {
        if (at<void *>(array, 0)) GameHeap_Free(at<void *>(array, 0));
        at<u32>(array, 0) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 4) = 0;
        return;
    }
    if (at<u32 *>(array, 0) == nullptr) {
        u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)newSize * 4));
        at<u32 *>(array, 0) = block;
        for (u32 i = 0; i < (u32)newSize; ++i) block[i] = 0;
        at<s32>(array, 8) = newSize;
        at<s32>(array, 4) = newSize;
        return;
    }
    const s32 max = at<s32>(array, 8);
    if (newSize <= max) {
        const s32 size = at<s32>(array, 4);
        if (newSize > size) {
            u32 *p = at<u32 *>(array, 0) + size;
            for (s32 n = newSize - size; n > 0; --n) *p++ = 0;
        }
        at<s32>(array, 4) = newSize;
        return;
    }
    s32 step = at<s32>(array, 0xc);
    if (step == 0) {
        step = at<s32>(array, 4);
        step = (step + ((step >> 31) & 7)) >> 3;
        if (step < 4) step = 4;
        else if (step > 0x400) step = 0x400;
    }
    s32 newMax = step + max;
    if (newSize >= newMax) newMax = newSize;
    u32 *const block = static_cast<u32 *>(GameHeap_Alloc((u32)newMax * 4));
    const s32 size = at<s32>(array, 4);
    const u32 *from = at<u32 *>(array, 0);
    for (s32 i = 0; i < size; ++i) block[i] = from[i];
    u32 *fill = block + size;
    for (s32 n = newSize - size; n > 0; --n) *fill++ = 0;
    GameHeap_Free(at<void *>(array, 0));
    at<s32>(array, 4) = newSize;
    at<u32 *>(array, 0) = block;
    at<s32>(array, 8) = newMax;
}

}  // extern "C"
