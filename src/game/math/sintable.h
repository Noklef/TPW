// The game's sine lookup table (gSinTable, 0x7b0968), used for most of its angle maths.
#pragma once

#include "../../../re/types.h"

// values[i] = sin(2 pi i / n); an angle a (radians) is looked up as
// values[round(a * scale) & mask], and its cosine at a + pi/2.
struct SinTable {
    const float *values;
    u32 mask;     // n - 1 (0x7b0960)
    float scale;  // n / (2 pi) (0x7b095c)
};

// 0x4708d0 (n = 4096) as the game first runs it: at the x87's default 53-bit precision,
// before Direct3D exists. values must hold n floats. Not hooked: the game builds the table
// again at shutdown, after Direct3D has lowered the precision, and that copy differs.
void SinTable_Build(float *values, u32 n, u32 *mask, float *scale);

// The live table inside the game process (hook entries only).
SinTable SinTable_Game();

// The entry for an angle already multiplied by scale (fistp, round to nearest even).
float SinTable_At(const SinTable &sines, float scaled);

// The renderer's own table: 256 floats at 0x8c63d8, values[i] = sin(i pi / 128), read by the
// rotation at 0x591430 (cosine at 0x40 - i).
extern "C" {
// 0x5913f0 (Engine_Init): fills it, fild i; fmul pi (a float, 0x702588); fmul 1/128; fsin.
void __cdecl SinTable256_Build();
}
