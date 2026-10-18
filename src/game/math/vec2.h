// A pair of floats made unit length: reimplementation of 0x5787b0 (the engine uses it on 0x87b080, the view's heading: the x and z
// of its forward row, 0x87afb0 and 0x87afd0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5787b0 (thiscall; x87): v times 1 / sqrt(y * y + x * x); a zero (or NaN) length squared is taken as 1, so a zero pair stays zero
// and a NaN one keeps its NaNs (the name is unconfirmed).
void __fastcall Vec2_Normalize(float *v);

// 0x549020 (cdecl; name unconfirmed): the length of the integer pair (x, y): their magnitudes (INT_MIN staying negative, sic) halved
// (arithmetic shifts) n times while |x| > 0x8000 or |y| > 0x7fff, the sum of their squares (32-bit; never negative, even from INT_MIN, whose
// square wraps to 0 or 2^30) square-rooted on the x87, truncated and shifted back left n times.
s32 __cdecl IntVec2_Length(s32 x, s32 y);
}
