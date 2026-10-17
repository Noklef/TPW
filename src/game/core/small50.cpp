#include "small50.h"

#include <cstdint>

#include "vobj3.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall Grid_AddObject(u8 *grid, void *, s32 x0, s32 y0, s32 x1, s32 y1, u8 *object) {
    u32 result = 1;
    const s32 width = at<s32>(grid, 0x38);
    const s32 height = at<s32>(grid, 0x3c);
    if (x0 >= width || x1 < 0 || y0 >= height || y1 < 0) return 0;
    if (x0 < 0) x0 = 0;
    if (x1 >= width) x1 = width - 1;
    if (y0 < 0) y0 = 0;
    if (y1 >= height) y1 = height - 1;
    for (s32 x = x0; x <= x1; ++x) {
        for (s32 y = y0; y <= y1; ++y)
            result &= Pool_AddSorted(grid, nullptr, reinterpret_cast<u8 **>(at<u8 *>(grid, 0x10) + (y * at<s32>(grid, 0x38) + x) * 4), object);
    }
    return result;
}

void __fastcall Surface_SetMasks(u8 *surface, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j, u32 k, u32 l) {
    at<u32>(surface, 0x80) = a;
    at<u32>(surface, 0x84) = b;
    at<u32>(surface, 0x88) = c;
    at<u32>(surface, 0x8c) = d;
    at<u32>(surface, 0x90) = e;
    at<u32>(surface, 0x94) = f;
    at<u32>(surface, 0x98) = g;
    at<u32>(surface, 0x9c) = h;
    at<u32>(surface, 0xa0) = i;
    at<u32>(surface, 0xa4) = j;
    at<u32>(surface, 0xa8) = k;
    at<u32>(surface, 0xac) = l;
    if (at<s32>(surface, 0x1c) != 0x10) return;
    const u8 low = (u8)f;      // (the third word of the second group)
    const u8 high = (u8)l;     // (the last word)
    for (u32 n = 0; n < 0x100; ++n) {
        at<u32>(surface, 0xb0 + 4 * n) = ((n >> ((8 - low) & 31)) << ((0x18 - low) & 31)) | ((n >> (low & 31)) << ((low + 8) & 31));
        const u32 mask = (1u << (high & 31)) - 1u;
        at<u32>(surface, 0x4b0 + 4 * n) = ((n & mask) << ((8 - high) & 31)) | ((n >> (high & 31)) << ((0x10 - (s8)i) & 31));
    }
}

}  // extern "C"
