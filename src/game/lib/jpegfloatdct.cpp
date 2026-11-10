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
    const float c0_707 = gJpegfloatdct_7058d0, c0_382 = gJpegfloatdct_7058cc, c0_541 = gJpegfloatdct_7058c8, c1_306 = gJpegfloatdct_7058c4;
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
        for (int i = 0; i < 64; ++i) out[i] = (s16)(ftol(X87Reg(workspace[i]) * divisors[i] + gJpeg6a_ForwardDctFloat_7058c0) - 0x4000);
    }
}

// The optimised 6b build keeps values on the x87 stack between statements: r<n> is a register's value, f<nn> the float stack slot at
// ESP+nn, written and read in the listing's order (a register and its stored float differ past the float range).
void __cdecl Jpeg6b_IdctFloat(u8 *cinfo, u8 *compptr, const s16 *coef_block, u8 **output_buf, u32 output_col) {
    const float k1_414 = gJpeg6b_IdctFloat_7050f8, k1_848 = gJpeg6b_IdctFloat_7050fc, k2_613 = gJpeg6b_IdctFloat_705100, k1_082 = gJpeg6b_IdctFloat_705104;
    const u8 *const limit = at<const u8 *>(cinfo, 0x120) + 0x80;
    float workspace[64];
    float f10, f14, f18, f1c, f20, f24, f28, f2c, f30;
    const s16 *c = coef_block;
    const float *q = at<const float *>(compptr, 0x50);
    float *w = workspace;
    for (int col = 8; col; --col, ++c, ++q, ++w) {
        if (!c[8] && !c[16] && !c[24] && !c[32] && !c[40] && !c[48] && !c[56]) {
            const float dc = (float)(X87Reg((double)c[0]) * q[0]);
            for (int k = 0; k < 64; k += 8) w[k] = dc;
            continue;
        }
        const X87Reg r1 = X87Reg(q[16]), r2 = X87Reg(q[48]), r3 = X87Reg(q[32]), r4 = X87Reg(q[8]);
        const X87Reg r5 = X87Reg((double)c[16]), r6 = X87Reg((double)c[48]), r7 = X87Reg((double)c[0]);
        const X87Reg r8 = r7 * q[0];
        f30 = (float)r8;
        const X87Reg r9 = r1 * r5, r10 = X87Reg(f30);
        f24 = (float)r9;
        const X87Reg r11 = X87Reg((double)c[32]), r12 = r3 * r11, r13 = r2 * r6, r14 = r12 + f30, r15 = r10 - r12;
        f20 = (float)r13;
        const X87Reg r16 = r9 - f20, r17 = r13 + f24;
        f10 = (float)r14;
        const X87Reg r18 = r16 * k1_414;
        f18 = (float)r17;
        const X87Reg r19 = r18 - f18, r20 = X87Reg(f10);
        f20 = (float)r15;
        f24 = (float)r19;
        const X87Reg r21 = r17 + f10, r22 = r20 - f18, r23 = r19 + f20, r24 = r15 - f24;
        f30 = (float)r21;
        f20 = (float)r22;
        f24 = (float)r23;
        f2c = (float)r24;
        const X87Reg r25 = X87Reg(q[24]), r26 = X87Reg(q[40]), r27 = X87Reg((double)c[8]), r28 = X87Reg((double)c[24]);
        const X87Reg r29 = X87Reg((double)c[40]), r30 = r4 * r27, r31 = r25 * r28;
        f18 = (float)r30;
        f1c = (float)r31;
        const X87Reg r32 = r26 * r29, r33 = X87Reg(q[56]), r34 = X87Reg(f18), r35 = X87Reg((double)c[56]), r36 = r33 * r35;
        const X87Reg r37 = r32 - f1c, r38 = r34 - r36, r39 = r32 + f1c;
        f14 = (float)r37;
        f1c = (float)r38;
        const X87Reg r40 = r38 + f14, r41 = r36 + f18, r42 = X87Reg(f14), r43 = r40 * k1_848;
        f28 = (float)r39;
        const X87Reg r44 = r42 * k2_613;
        f10 = (float)r41;
        const X87Reg r45 = r39 + f10, r46 = X87Reg(f10);
        f14 = (float)r43;
        const X87Reg r47 = f14 - r44, r48 = r46 - f28;
        f28 = (float)r45;
        const X87Reg r49 = r47 - f28, r50 = X87Reg(f1c), r51 = r48 * k1_414;
        f10 = (float)r49;
        const X87Reg r52 = r50 * k1_082, r53 = r51 - f10, r54 = r52 - f14, r55 = X87Reg(f28), r56 = X87Reg(f30);
        f1c = (float)r53;
        const X87Reg r57 = r54 + f1c, r58 = X87Reg(f10), r59 = X87Reg(f24), r60 = X87Reg(f1c);
        f18 = (float)r57;
        w[0] = (float)(r55 + f30);
        const X87Reg r62 = X87Reg(f2c);
        w[56] = (float)(r56 - f28);
        const X87Reg r64 = X87Reg(f20);
        w[8] = (float)(r58 + f24);
        w[48] = (float)(r59 - f10);
        w[16] = (float)(r60 + f2c);
        w[40] = (float)(r62 - f1c);
        w[32] = (float)(r57 + f20);
        w[24] = (float)(r64 - f18);
    }
    w = workspace;
    for (int row = 0; row < 8; ++row, w += 8) {
        const X87Reg r1 = X87Reg(w[5]), r2 = X87Reg(w[1]), r3 = r2 - w[7], r4 = r1 - w[3], r5 = X87Reg(w[3]), r6 = r5 + w[5];
        f1c = (float)r3;
        const X87Reg r7 = X87Reg(w[7]), r8 = r3 + r4, r9 = r7 + w[1], r10 = r8 * k1_848;
        f28 = (float)r6;
        const X87Reg r11 = r4 * k2_613, r12 = X87Reg(w[2]);
        f10 = (float)r9;
        const X87Reg r13 = r6 + f10, r14 = X87Reg(f10);
        f14 = (float)r10;
        const X87Reg r15 = f14 - r11, r16 = r14 - f28, r17 = X87Reg(w[6]), r18 = r12 - w[6], r19 = r17 + w[2], r20 = X87Reg(w[0]);
        const X87Reg r21 = r20 + w[4];
        u8 *const o = output_buf[row] + output_col;
        f10 = (float)r21;
        const X87Reg r22 = X87Reg(w[0]), r23 = r22 - w[4];
        f20 = (float)r23;
        f18 = (float)r19;
        const X87Reg r24 = r18 * k1_414, r25 = r24 - f18;
        f24 = (float)r25;
        const X87Reg r26 = r19 + f10;
        f30 = (float)r26;
        f28 = (float)r13;
        const X87Reg r27 = r15 - f28, r28 = X87Reg(f1c), r29 = r16 * k1_414;
        f10 = (float)r27;
        const X87Reg r30 = r28 * k1_082, r31 = r29 - f10, r32 = r30 - f14, r33 = X87Reg(f20), r34 = r21 - f18, r35 = r25 + f20;
        const X87Reg r36 = r33 - f24, r37 = X87Reg(f28);
        f20 = (float)r34;
        f1c = (float)r31;
        const X87Reg r38 = r37 + f30, r39 = r32 + f1c;
        f24 = (float)r35;
        f2c = (float)r36;
        f18 = (float)r39;
        // (DESCALE by 3 of each __ftol, through the range limit)
        o[0] = limit[((ftol(r38) + 4) >> 3) & 0x3ff];
        o[7] = limit[((ftol(X87Reg(f30) - f28) + 4) >> 3) & 0x3ff];
        o[1] = limit[((ftol(X87Reg(f10) + f24) + 4) >> 3) & 0x3ff];
        o[6] = limit[((ftol(X87Reg(f24) - f10) + 4) >> 3) & 0x3ff];
        o[2] = limit[((ftol(X87Reg(f1c) + f2c) + 4) >> 3) & 0x3ff];
        o[5] = limit[((ftol(X87Reg(f2c) - f1c) + 4) >> 3) & 0x3ff];
        o[4] = limit[((ftol(X87Reg(f18) + f20) + 4) >> 3) & 0x3ff];
        o[3] = limit[((ftol(X87Reg(f20) - f18) + 4) >> 3) & 0x3ff];
    }
}

}  // extern "C"
