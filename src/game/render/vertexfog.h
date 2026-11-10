// Fog applied to lit vertices by their depth: reimplementation of 0x5747d0 and 0x574960 (stdcall, x87 at the game's 24-bit
// precision; called by the mesh drawing at 0x5655c9 and its kind). The names are unconfirmed.
//
// The fog (see src/game/render/state): 0x8bd47c its colour, 0x8bd484 / 0x8bd488 / 0x8bd48c the colour's red, green and blue as floats
// (0-255), 0x8bd490 and 0x8bd494 where it starts and ends, 0x8bd498 the fraction per unit between them. A vertex: its depth at +8,
// its diffuse colour (ARGB) at +0x10 and its specular at +0x14. Each float made an integer goes through the dword 0xfb34a4 (fistp,
// rounding to nearest), which keeps the last.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5747d0 (vertices, count, stride in bytes; pops 0xc): for each vertex, with z its depth: z at or before the start (or a NaN):
// specular = 0. z at or past the end: the diffuse keeps only its alpha, specular = the fog colour. Between: t = (z - start) *
// fraction (as a float); specular = the fog's red * t, green * t and blue * t rounded and masked into place, and the diffuse's red,
// green and blue (each kept in place, a multiple of its unit) times 1 - t, rounded and masked into place, its alpha kept.
void __stdcall VertexFog_Colour(u8 *vertices, u32 count, s32 stride);

// 0x574960 (vertices, count, stride; pops 0xc): the specular alone, the fog in its alpha: z at or before the start (or a NaN):
// 0xff000000; at or past the end: 0; between: ((end - z) * fraction * 255, as a float) rounded, shifted to the top byte.
void __stdcall VertexFog_Alpha(u8 *vertices, u32 count, s32 stride);
}
