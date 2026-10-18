#include "fixedverts.h"

#include <cstdint>
#include <xmmintrin.h>

#include "../math/x87.h"

namespace {

template <typename T> T &at(void *p, s32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

// fld m32; fmul m32 (the 24-bit precision rounding the product); __ftol
s32 Scaled(float f, float scale) {
    return (s32)(long long)(X87Reg(f) * scale).v;
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// fld m32; fmul; fstp m32 (into the count's argument slot, sic); fld; fistp into the dword at slot, which keeps the last: rounded to
// nearest even, 0x80000000 when out of range (as cvtss2si does)
s32 Rounded(float f, float scale, u32 slot) {
    const float product = (float)(X87Reg(f) * scale);
    const s32 n = _mm_cvtss_si32(_mm_set_ss(product));
    global<s32>(slot) = n;
    return n;
}

}  // namespace

extern "C" {

void __cdecl FixedVerts_Pack(u8 *verts, u32 count, s32 stride) {
    if (count == 0) return;
    u8 *src = verts, *dst = verts;
    do {
        at<s32>(dst, 0) = Scaled(at<float>(src, 0), 32.0f);  // 0x70114c
        at<s32>(dst, 4) = Scaled(at<float>(src, 4), 16.0f);  // 0x701144
        at<s32>(dst, 8) = Scaled(at<float>(src, 8), 65535.0f);  // 0x701150
        at<u32>(dst, 0xc) = at<u32>(src, 0x10);
        at<s32>(dst, 0x10) = Scaled(at<float>(src, 0x18), 16.0f);
        at<s32>(dst, 0x14) = Scaled(at<float>(src, 0x1c), 16.0f);
        src += stride;
        dst += 0x18;
    } while (--count);
}

void __cdecl FixedVerts_PackRounded(u8 *verts, u32 count, s32 stride) {
    const float xy = (global<u32>(0x008bd508) & 0x80000020) == 0x80000020 ? 8.0f : 16.0f;
    if (count == 0) return;
    u8 *src = verts, *dst = verts;
    do {
        at<s32>(dst, 0) = Rounded(at<float>(src, 0), xy, 0x00fb34a0);
        at<s32>(dst, 4) = Rounded(at<float>(src, 4), xy, 0x00fb34a0);
        at<s32>(dst, 8) = Rounded(at<float>(src, 8), 32768.0f, 0x00fb34a4);  // 0x701148
        at<u32>(dst, 0xc) = at<u32>(src, 0x10);
        at<s32>(dst, 0x10) = Rounded(at<float>(src, 0x18), 16.0f, 0x00fb34a4);  // 0x701144
        at<s32>(dst, 0x14) = Rounded(at<float>(src, 0x1c), 16.0f, 0x00fb34a4);
        src += stride;
        dst += 0x18;
    } while (--count);
}
}
