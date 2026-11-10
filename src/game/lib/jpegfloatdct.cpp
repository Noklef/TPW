// libjpeg's float DCTs as the exe's x87 code runs them (see jpegfloatdct.h).
#include "jpegfloatdct.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
inline float rdata(u32 address) { return *reinterpret_cast<const float *>((uintptr_t)address); }

// __ftol (0x67a830): truncated to 64 bits, the low half (0 for a NaN or past 2^63).
inline s32 ftol(X87Reg x) { return (s32)(long long)x.v; }

// One pass of jfdctflt.c over the 8 rows (step 1, next 8) or the 8 columns (step 8, next 1), in the unoptimised 6a build's order:
// every variable a float local, each statement loaded, operated on and stored, except where one stays in the register (FST then on).
void fdct_pass(float *p, int s, int next) {
    const float c0_707 = rdata(0x007058d0), c0_382 = rdata(0x007058cc), c0_541 = rdata(0x007058c8), c1_306 = rdata(0x007058c4);
    for (int ctr = 7; ctr >= 0; --ctr, p += next) {
        const float tmp0 = (float)(X87Reg(p[0]) + p[7 * s]), tmp7 = (float)(X87Reg(p[0]) - p[7 * s]);
        const float tmp1 = (float)(X87Reg(p[s]) + p[6 * s]), tmp6 = (float)(X87Reg(p[s]) - p[6 * s]);
        const float tmp2 = (float)(X87Reg(p[2 * s]) + p[5 * s]), tmp5 = (float)(X87Reg(p[2 * s]) - p[5 * s]);
        const float tmp3 = (float)(X87Reg(p[3 * s]) + p[4 * s]), tmp4 = (float)(X87Reg(p[3 * s]) - p[4 * s]);
        float tmp10 = (float)(X87Reg(tmp0) + tmp3);
        const float tmp13 = (float)(X87Reg(tmp0) - tmp3);
        float tmp11 = (float)(X87Reg(tmp1) + tmp2), tmp12 = (float)(X87Reg(tmp1) - tmp2);
        p[0] = (float)(X87Reg(tmp10) + tmp11);
        p[4 * s] = (float)(X87Reg(tmp10) - tmp11);
        const float z1 = (float)((X87Reg(tmp12) + tmp13) * c0_707);
        p[2 * s] = (float)(X87Reg(tmp13) + z1);
        p[6 * s] = (float)(X87Reg(tmp13) - z1);
        tmp10 = (float)(X87Reg(tmp4) + tmp5);
        tmp11 = (float)(X87Reg(tmp5) + tmp6);
        tmp12 = (float)(X87Reg(tmp6) + tmp7);
        const float z5 = (float)((X87Reg(tmp10) - tmp12) * c0_382);
        const float z2 = (float)(X87Reg(c0_541) * tmp10 + z5);
        const float z4 = (float)(X87Reg(c1_306) * tmp12 + z5);
        const float z3 = (float)(X87Reg(tmp11) * c0_707);
        const float z11 = (float)(X87Reg(tmp7) + z3);
        const X87Reg r13 = X87Reg(tmp7) - z3;  // (FST z13: the register goes on)
        const float z13 = (float)r13;
        p[5 * s] = (float)(r13 + z2);
        p[3 * s] = (float)(X87Reg(z13) - z2);
        p[s] = (float)(X87Reg(z11) + z4);
        p[7 * s] = (float)(X87Reg(z11) - z4);
    }
}

}  // namespace

extern "C" {

void __cdecl Jpeg6a_FdctFloat(float *data) {
    fdct_pass(data, 1, 8);
    fdct_pass(data, 8, 1);
}

void __cdecl Jpeg6a_ForwardDctFloat(u8 *cinfo, u8 *compptr, u8 **sample_data, s16 *coef_blocks, u32 start_row, u32 start_col,
                                    u32 num_blocks) {
    u8 *const fdct = at<u8 *>(cinfo, 0x160);
    void(__cdecl *const do_dct)(float *) = at<void(__cdecl *)(float *)>(fdct, 0x1c);
    const float *const divisors = at<const float *>(fdct, 0x20 + 4 * at<s32>(compptr, 0x10));
    sample_data += start_row;
    for (u32 bi = 0; bi < num_blocks; ++bi, start_col += 8) {
        float workspace[64];
        float *w = workspace;
        for (int row = 0; row < 8; ++row)
            for (int k = 0; k < 8; ++k) *w++ = (float)(s32)(sample_data[row][start_col + k] - 0x80);
        do_dct(workspace);
        s16 *const out = coef_blocks + 64 * bi;
        for (int i = 0; i < 64; ++i) out[i] = (s16)(ftol(X87Reg(workspace[i]) * divisors[i] + rdata(0x007058c0)) - 0x4000);
    }
}

}  // extern "C"
