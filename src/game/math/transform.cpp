#include "transform.h"

#include <cstdint>

#include "x87.h"
#include "../core/trivial.h"

namespace {

float *Step(float *p, s32 bytes) { return reinterpret_cast<float *>((uintptr_t)p + (u32)bytes); }
const float *Step(const float *p, s32 bytes) { return reinterpret_cast<const float *>((uintptr_t)p + (u32)bytes); }

template <typename T> T &At(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kView = 0x8bcbc8;  // the render view (src/game/render/viewport)

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

double __cdecl Transform_ProjectToScreen(void *, float *v, s32 stride, u32 count, s32 mode) {
    X87Reg sum = 0.0f;  // 0x701978
    for (; count != 0; --count, v = Step(v, stride)) {
        const u32 z_bits = At<u32>((uintptr_t)&v[2]);
        const X87Reg r = X87Reg(1.0f) / v[3];  // 0x701974
        v[3] = (float)r;
        v[0] = (float)((r * At<float>(kView + 0x38)) * v[0] + At<float>(kView + 0x28));
        v[1] = (float)((r * At<float>(kView + 0x3c)) * v[1] + At<float>(kView + 0x2c));
        if (mode == 1) {
            v[2] = X87_Through(At<float>(kView + 0x74));
        } else if (mode == 2) {
            v[2] = (float)(X87Reg(At<float>(kView + 0x58)) * v[2]);
        } else if (z_bits < At<u32>(kView + 0x60)) {
            v[2] = (float)(X87Reg(At<float>(kView + 0x6c)) * v[2] + At<float>(kView + 0x70));
        } else {
            v[2] = (float)((r * At<float>(kView + 0x64)) * v[2] + At<float>(kView + 0x68));
        }
        sum = sum + v[2];
    }
    return sum.v;
}

void __cdecl Transform_Concatenate(void *, const float *a, const float *b, float *out) {
    for (int r = 0; r < 16; r += 4) {
        X87Reg c[4] = {0.0, 0.0, 0.0, 0.0};
        for (int k = 0; k < 4; ++k)
            c[k] = ((X87Reg(a[r]) * b[k] + X87Reg(a[r + 1]) * b[4 + k]) + X87Reg(a[r + 2]) * b[8 + k]) + X87Reg(a[r + 3]) * b[12 + k];
        out[r] = (float)c[0];
        out[r + 2] = (float)c[2];
        out[r + 1] = (float)c[1];
        out[r + 3] = (float)c[3];
    }
}

u32 __cdecl Transform_SseProbe() {
    volatile u32 result = 0;  // (the original's [EBP-0x1c], which its filter reads; there it is not set before the call)
    __try {
        result = ReturnOne();
    } __except (result != 1 ? 1 : 0) {  // (EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        return 0;
    }
    return 1;
}
}
