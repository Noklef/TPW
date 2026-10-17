#include "transform.h"

#include <cstdint>

#include "x87.h"

namespace {

float *Step(float *p, s32 bytes) { return reinterpret_cast<float *>((uintptr_t)p + (u32)bytes); }
const float *Step(const float *p, s32 bytes) { return reinterpret_cast<const float *>((uintptr_t)p + (u32)bytes); }

}  // namespace

extern "C" {

void __cdecl Transform_BlockedPoints(void *, const float *m, float *out, const u8 *in, s32 out_stride, u32 first, u32 count) {
    u32 slot = first & 3;
    const u8 *block = in + ((first * 4) & 0x10) * 3;
    for (; count != 0; --count) {
        const float *v = reinterpret_cast<const float *>(block) + slot;
        ++slot;
        const float x = v[0], y = v[4], z = v[8];
        const X87Reg w_ = ((X87Reg(m[7]) * y + X87Reg(m[3]) * x) + X87Reg(m[11]) * z) + m[15];
        const X87Reg y_ = ((X87Reg(m[5]) * y + X87Reg(m[1]) * x) + X87Reg(m[9]) * z) + m[13];
        const X87Reg x_ = (X87Reg(m[8]) * z + (X87Reg(m[4]) * y + X87Reg(m[0]) * x)) + m[12];
        const X87Reg z_ = (X87Reg(m[10]) * z + (X87Reg(m[6]) * y + X87Reg(m[2]) * x)) + m[14];
        block += ((slot * 4) & 0x10) * 3;
        slot &= 3;
        out[3] = (float)w_;
        out[1] = (float)y_;
        out[0] = (float)x_;
        out[2] = (float)z_;
        out = Step(out, out_stride);
    }
}

void __cdecl Transform_Vertices(void *, const float *m, float *out, const u8 *in_bytes, s32 out_stride, s32 in_stride, u32 count,
                                s32 mode) {
    const float *in = reinterpret_cast<const float *>(in_bytes);
    for (; count != 0; --count, in = Step(in, in_stride), out = Step(out, out_stride)) {
        const float x = in[0], y = in[1], z = in[2];
        if (mode == 1) {
            const float w = in[3];
            out[0] = (float)(((X87Reg(y) * m[4] + X87Reg(w) * m[12]) + X87Reg(z) * m[8]) + X87Reg(m[0]) * x);
            out[1] = (float)(((X87Reg(w) * m[13] + X87Reg(y) * m[5]) + X87Reg(x) * m[1]) + X87Reg(z) * m[9]);
            out[2] = (float)(((X87Reg(z) * m[10] + X87Reg(y) * m[6]) + X87Reg(x) * m[2]) + X87Reg(w) * m[14]);
            out[3] = (float)(((X87Reg(z) * m[11] + X87Reg(y) * m[7]) + X87Reg(w) * m[15]) + X87Reg(x) * m[3]);
            continue;
        }
        const X87Reg x_ = (X87Reg(y) * m[4] + X87Reg(z) * m[8]) + X87Reg(m[0]) * x;
        const X87Reg y_ = (X87Reg(y) * m[5] + X87Reg(x) * m[1]) + X87Reg(z) * m[9];
        const X87Reg z_ = (X87Reg(z) * m[10] + X87Reg(y) * m[6]) + X87Reg(x) * m[2];
        if (mode == 3) {
            out[0] = (float)x_;
            out[1] = (float)y_;
            out[2] = (float)z_;
            continue;
        }
        out[0] = (float)(x_ + m[12]);
        out[1] = (float)(y_ + m[13]);
        out[2] = (float)(z_ + m[14]);
        if (mode != 2) out[3] = (float)(((X87Reg(z) * m[11] + X87Reg(y) * m[7]) + X87Reg(x) * m[3]) + m[15]);
    }
}
}
