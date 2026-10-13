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
