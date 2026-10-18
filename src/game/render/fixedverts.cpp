#include "fixedverts.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(void *p, s32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

// fld m32; fmul m32 (the 24-bit precision rounding the product); __ftol
s32 Scaled(float f, float scale) {
    return (s32)(long long)(X87Reg(f) * scale).v;
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
}
