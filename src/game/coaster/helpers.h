// Small float helpers next to the coaster track code.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43e160: (int)x truncated, plus one if x had a fraction; so 1.5 -> 2 but -1.5 -> 0 (not
// ceil). Through the CRT's modf and __ftol: a NaN or an x beyond the int64 range gives
// 0x80000000_00000000's low dword (0), and a NaN fraction counts as none.
int __cdecl Float_RoundUp(float x);

// 0x43e480: the angle t of the way from a to b, both in radians. Within pi of each other it
// is the plain lerp b*t + (1-t)*a. Further apart it goes the other way round, by
// gap = 2pi - |a - b| (stored as a float): from b, (1-t)*gap + b when a < 0 (or NaN), else
// from a, gap*t + a; that is then put in -pi..pi as fmod(x + pi, 2pi) - pi, with pi and 2pi
// as doubles there (the floats' values). Left in st0 at the game's precision, so double.
// sic: the wrap goes from b when a < 0 but from a otherwise, so at t = 0 the result is a
// only for a >= 0 (the coaster car's heading between two track frames).
double __cdecl Angle_Lerp(float a, float b, float t);

// 0x43a970: a . b, as (a.y b.y + a.z b.z) + a.x b.x. Left in st0, so double.
double __cdecl Vec3_Dot(const float *a, const float *b);

// 0x43a9a0: v over its length (rounded to a float first), in place.
// sic: a zero vector becomes NaNs.
void __cdecl Vec3_Normalize(float *v);

// 0x4387a0: v over the length of its x and z alone (rounded to a float first), y too, in
// place. sic: a vertical vector becomes NaNs or infinities.
void __cdecl Vec3_NormalizeXZ(float *v);
}
