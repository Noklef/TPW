// The sine lookup table: reimplementation of 0x4708d0 and of the game's lookups.
#include "sintable.h"

#include <cmath>
#include <xmmintrin.h>

#include "x87.h"

void SinTable_Fill(float *values, u32 n, u32 *mask, float *scale) {
    const float nf = (float)n;
    *mask = n - 1;
    *scale = nf * 0.159154937f;  // 0x6feb9c; exact in double, so one rounding to float
    for (u32 i = 0; i < n; ++i)
        values[i] = (float)std::sin((double)i / nf * 6.28318548f);  // 2 pi as a float (0x6feba0)
}

SinTable SinTable_Game() {
    return {reinterpret_cast<const float *>(0x007b0968), *reinterpret_cast<const u32 *>(0x007b0960),
            *reinterpret_cast<const float *>(0x007b095c)};
}

float SinTable_At(const SinTable &sines, float scaled) {
    // fistp rounds to nearest even; out of range gives 0x80000000, as cvtss2si does.
    return sines.values[(u32)_mm_cvtss_si32(_mm_set_ss(scaled)) & sines.mask];
}

extern "C" {

void __cdecl SinTable_Build(u32 n) {
    u16 cw;
    __asm fnstcw cw
    const bool single = (cw & 0x300) == 0;  // (24-bit precision; else 53, as double arithmetic rounds)
    float *const values = reinterpret_cast<float *>(0x007b0968);
    *reinterpret_cast<u32 *>(0x007b0960) = n - 1;
    *reinterpret_cast<u32 *>(0x007b4968) = n;
    const float nf = (float)(double)n;  // (fild qword n, exact; fst float)
    *reinterpret_cast<float *>(0x007b095c) = single ? (float)X87_MulInt(n, 0.159154937f) : (float)((double)n * (double)0.159154937f);
    for (u32 i = 0; i < n; ++i)
        values[i] = (float)X87_Sin(single ? (X87Reg((double)i) / X87Reg(nf) * X87Reg(6.28318548f)).v : (double)i / nf * (double)6.28318548f);
}

void __cdecl SinTable_BuildGame() { SinTable_Build(0x1000); }

void __cdecl SinTable256_Build() {
    float *values = reinterpret_cast<float *>(0x008c63d8);
    for (s32 i = 0; i < 0x100; ++i)
        values[i] = (float)X87_Sin((X87Reg((double)i) * 3.14159274f * 0.0078125f).v);
}
}
