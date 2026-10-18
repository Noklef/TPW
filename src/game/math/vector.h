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
}
