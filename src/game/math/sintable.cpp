// The sine lookup table: reimplementation of 0x4708d0 and of the game's lookups.
#include "sintable.h"

#include <cmath>
#include <xmmintrin.h>

#include "x87.h"

void SinTable_Build(float *values, u32 n, u32 *mask, float *scale) {
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

void __cdecl SinTable256_Build() {
    float *values = reinterpret_cast<float *>(0x008c63d8);
    for (s32 i = 0; i < 0x100; ++i)
        values[i] = (float)X87_Sin((X87Reg((double)i) * 3.14159274f * 0.0078125f).v);
}
}
