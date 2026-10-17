#include "clipcodes.h"

#include <cstdint>

namespace {

template <typename T> T at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall ClipCodes_Compute(u32 ecx, u32, u8 *out, u8 *codes, const u8 *points, u32 count, u32 stride, u32 mask) {
    const u8 *const table = reinterpret_cast<const u8 *>((uintptr_t)0x00879970);
    u8 all = 0xff;
    u8 any = 0;
    do {
        const u32 x = at<u32>(points, 0), y = at<u32>(points, 4), z = at<u32>(points, 8), w = at<u32>(points, 0xc);
        const u32 x2 = x << 1, y2 = y << 1, z2 = z << 1, w2 = w << 1;
        u32 index = w >> 31;
        index = index << 1 | z >> 31;
        index = index << 1 | (w2 < z2 ? 1u : 0u);
        index = index << 1 | y >> 31;
        index = index << 1 | (w2 < y2 ? 1u : 0u);
        index = index << 1 | x >> 31;
        index = index << 1 | (w2 < x2 ? 1u : 0u);
        const u8 code = table[index] & (u8)mask;
        all &= code;
        *codes++ = code;
        any |= code;
        points += stride;
    } while (--count != 0);
    *reinterpret_cast<u32 *>(out) = (ecx & 0xffff0000u) | (u32)any << 8 | all;  // sic: the top half is the caller's ECX
    return out;
}

u8 *__fastcall ClipCodes_Run(u32 ecx, u32, u8 *out, u8 *codes, const u8 *points, u32 count, u32 stride, u32 mask) {
    ClipCodes_Compute(ecx, (u32)(uintptr_t)points, out, codes, points, count, stride, mask);
    return out;
}
}
