#include "bounds.h"

#include <cstdint>

#include "../math/x87.h"

extern "C" {

void __cdecl Model_TransformBounds(const u8 *source, u8 *model) {
    const u32 *const from = reinterpret_cast<const u32 *>(source + 0x80);
    u32 *const to = reinterpret_cast<u32 *>(model + 0x80);
    for (u32 i = 0; i < 6; ++i) to[i] = from[i];  // rep movsd
    for (u32 k = 0; k < 2; ++k) {
        const float *const m = reinterpret_cast<const float *>(*reinterpret_cast<u8 *const *>(model + 0x78) + 0x10);
        float *const p = reinterpret_cast<float *>(model + 0x80 + 0xc * k);
        const float x = p[0], y = p[1], z = p[2];
        const X87Reg ny = X87Reg(m[9]) * z + X87Reg(m[5]) * y + X87Reg(m[1]) * x + m[13];
        const X87Reg nz = X87Reg(m[10]) * z + X87Reg(m[6]) * y + X87Reg(m[2]) * x + m[14];
        const X87Reg nx = X87Reg(m[8]) * z + X87Reg(m[4]) * y + X87Reg(x) * m[0] + m[12];
        p[0] = (float)nx;
        p[1] = (float)ny;
        p[2] = (float)nz;
    }
}
}
