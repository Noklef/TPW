#include "clipcodes.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "engineerrors.h"

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

u32 __cdecl ClipCodes_InitTable() {
    u8 **const block = reinterpret_cast<u8 **>((uintptr_t)0x00879a20);
    if (*block) return 0;
    u8 *const table = reinterpret_cast<u8 *>((uintptr_t)0x00879970);
    for (s32 i = 0; i < 0x80; ++i) {
        const float x = (float)((1 - 2 * (i >> 1 & 1)) * (3 * (i & 1) + 1));
        const float y = (float)((1 - 2 * (i >> 3 & 1)) * (3 * (i >> 2 & 1) + 1));
        const float z = (float)((1 - 2 * (i >> 5 & 1)) * (3 * (i >> 4 & 1) + 1));
        const float w = (float)(2 - 4 * (i >> 6 & 1));
        u8 code = (u8)((y > w) << 5);
        code |= (u8)((y < -w) << 4);
        code |= (u8)((x > w) << 3);
        code |= (u8)((x < -w) << 2);
        code |= (u8)((z > w) << 1);
        code |= (u8)(z < 0.0f);
        table[i] = code;
    }
    *block = static_cast<u8 *>(GameHeap_Alloc(8));
    if (*block) return 1;
    EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
    return 0;
}
}
