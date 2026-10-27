// Small vector helpers (0x431480-0x431660) next to the track-orientation code that uses
// them. Expression trees from the asm, as for the matrix library (matrix.h).
#pragma once

#include "../../../re/types.h"
#include "sintable.h"

// 0x4314f0, 0x431570 and 0x4315f0 with the table given.
void Vec3_RotateXWith(const SinTable &sines, float *v, float angle);
void Vec3_RotateYWith(const SinTable &sines, float *v, float angle);
void Matrix_RotateYAngleWith(const SinTable &sines, float *m, float angle);

extern "C" {
// 0x431480: the fraction of x (CRT modf, exact); the whole part goes to *whole as a float
// when whole is not null. The original leaves the double in st0.
double __cdecl Float_Modf(float x, float *whole);
// 0x4314b0: v about y by (cos, sin), in place: the per-row step of Matrix_RotateY.
void __cdecl Vec3_RotateYCS(float *v, float c, float s);
// 0x4314f0, 0x431570: v about x / y by angle (radians), cos and sin from gSinTable.
void __cdecl Vec3_RotateX(float *v, float angle);
void __cdecl Vec3_RotateY(float *v, float angle);
// 0x4315f0: Matrix_RotateY(m, cos angle, sin angle) from gSinTable.
void __cdecl Matrix_RotateYAngle(float *m, float angle);
// 0x5ebe50 (thiscall; name unconfirmed; its one caller clamps a colour scaled by 255 to 0..255): each of v's three floats above hi becomes
// hi, else one not at or above lo (below it, or NaN, or lo NaN) becomes lo; each stored through the x87 (a signalling NaN quietened).
void __fastcall Vec3_Clamp(float *v, void *, float lo, float hi);
// 0x6bb240 (thiscall (scale), pops 4; name unconfirmed; the sound channel's settings use it): each of v's three floats times the scale
// (v[0] * s, s * v[1], s * v[2] in the x87's operand order, all read before the first store), stored back.
void __fastcall Vec3_Scale(float *v, void *, float scale);
// 0x6c4f00 (name unconfirmed; the sound occluders' test uses it): x clamped to the interval between a and b, whichever is the larger. With
// a below b (or either NaN): x above b gives b, else x at or above a gives x, else a; otherwise x above a gives a, else x at or above b
// gives x, else b. Returned in st0 (loaded with fld: a signalling NaN quietened).
float __cdecl Float_ClampBetween(float x, float a, float b);
// 0x565110 (thiscall; angle; name unconfirmed; called only by the static initialiser 0x5650f0, on 0x879a70 after its +0 = 100.0): three
// directions in the xy plane after the dword +0: (sin, cos, 0) of the angle at +4, of pi times it at +0x10, and of pi times (it + 1) at
// +0x1c (the x87's fsin / fcos; the products rounded to the precision control).
void __fastcall Vec3_DirectionsFromAngle(float *out, void *, float angle);
}
