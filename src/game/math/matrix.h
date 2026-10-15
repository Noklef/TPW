// The engine's matrix library (0x46f150-0x4709d0): 4x4 float matrices in Direct3D's row
// layout, m[0..2] the x axis, m[4..6] y, m[8..10] z and m[12..14] the translation. The
// expression trees are the original's (tools/x87sym.py); see curve.h for why that matters.
#pragma once

#include "../../../re/types.h"
#include "sintable.h"

// 0x46f6f0 with the table given.
void Matrix_RotateEulerWith(const SinTable &sines, float *m, s32 order, const float *angles);

extern "C" {
// 0x46f150
void __cdecl Matrix_Identity(float *m);
// 0x46f180: identity axes, translation (m[12..14]) kept.
void __cdecl Matrix_ResetRotation(float *m);

// Angles by fpatan. The original returns them at full x87 precision; these return the
// double, which every caller rounds to float.
// 0x46f1c0: heading of the z axis about y, atan2(m8, m10); from the x axis,
// atan2(-m2, m0), when the z axis is within about 25 degrees of vertical (|m9| > 0.9).
double __cdecl Matrix_Heading(const float *m);
// 0x46f200: atan2(-m4, m5)
double __cdecl Matrix_Roll(const float *m);
// 0x46f210, 0x46f220, 0x46f230: atan2(-y, z), atan2(x, z), atan2(-x, y)
double __cdecl Vec3_AngleX(const float *v);
double __cdecl Vec3_AngleY(const float *v);
double __cdecl Vec3_AngleZ(const float *v);

// Rotation by (cos, sin) about an axis, in place. The 4 forms rotate the translation too
// (0x46f240 x, 0x46f420 y, 0x46f4f0 z); the others only the three axes (0x46f350 x,
// 0x46f650 y, 0x46f5c0 z).
void __cdecl Matrix_RotateX4(float *m, float c, float s);
void __cdecl Matrix_RotateY4(float *m, float c, float s);
void __cdecl Matrix_RotateZ4(float *m, float c, float s);
void __cdecl Matrix_RotateX(float *m, float c, float s);
void __cdecl Matrix_RotateY(float *m, float c, float s);
void __cdecl Matrix_RotateZ(float *m, float c, float s);

// Rotations about the axes named by gEulerOrders[order] ("X" .. "ZYX"; entry 0 is a
// placeholder that rotates nothing), one angle (radians) per letter. 0x46f6f0 takes sines
// and cosines from gSinTable, 0x46f870 from fsin/fcos.
void __cdecl Matrix_RotateEulerFast(float *m, s32 order, const float *angles);
void __cdecl Matrix_RotateEuler(float *m, s32 order, const float *angles);

// 0x46f910: rescale the axes to the given lengths.
void __cdecl Matrix_SetAxisLengths(float *m, const float *x, const float *y, const float *z);
// 0x46fa70, 0x46fad0: multiply the axes by s[0], s[1], s[2] / by x, y, z.
void __cdecl Matrix_ScaleAxesV(float *m, const float *s);
void __cdecl Matrix_ScaleAxes(float *m, float x, float y, float z);
// 0x46fb50: the axes of a rotation by pitch about x, then by yaw about y (fsin/fcos);
// m[3], m[7] and m[11..15] are left alone.
void __cdecl Matrix_FromPitchYaw(float *m, float pitch, float yaw);
// 0x46fbb0: copy the given axes in (a null one is left alone).
void __cdecl Matrix_SetAxes(float *m, const float *x, const float *y, const float *z);
// 0x46fc00, 0x46fca0: v = v * m, in place, with and without the translation.
void __cdecl Matrix_TransformPoint(const float *m, float *v);
void __cdecl Matrix_TransformVector(const float *m, float *v);

// Products for row vectors (v * a * b applies a first). Only the 3x4 affine part is
// computed; m[3], m[7], m[11] and m[15] of the result are left as they were.
// 0x4700f0: a = a * b, stored by rows.
void __cdecl Matrix_Mul(float *a, const float *b);
// 0x4702d0: b = a * b, stored by columns.
void __cdecl Matrix_PreMul(const float *a, float *b);
// 0x4704c0: b = a * b for the 3x3 rotation parts only, stored by columns.
void __cdecl Matrix_PreMul3(const float *a, float *b);

// 0x470d10: m = r * m on the rotation part, r a packed 3x3 (9 floats, row major).
void __cdecl Matrix_PreMul33(const float *r, float *m);

// Numerical Recipes' ludcmp (0x46fe20) and lubksb (0x46fd40) for n = 4, without the
// singular-matrix error: a zero pivot becomes 1e-20.
void __cdecl Matrix_LUDecompose(float *a, s32 *index, float *parity);
void __cdecl Matrix_LUBackSubstitute(const float *a, const s32 *index, float *b);
// 0x470040: dst = src^-1 by LU decomposition, one column at a time; dst may be src.
void __cdecl Matrix_Invert(float *dst, const float *src);

// 0x470630: stable merge sort of n offsets by the float at keys + offset, ascending.
// Call with depth 0: the result is in b, and a is left in pieces.
void __cdecl Sort_MergeByKey(s32 n, s32 *a, s32 *b, s32 depth, const u8 *keys);

// 0x470780: point the z axis along dir, keeping the y axis as the up hint:
// x = normalize(y x dir), y = dir x x, z = dir (none of it normalised but x).
void __cdecl Matrix_LookAlong(float *m, const float *dir);

// 0x470940: |a - b|, left in st0 at the 24-bit precision with the register's exponent, so
// double: sqrt((dx*dx + dy*(float)dy) + (float)dz*(float)dz), dx squared in the register
// (through a double store), dy and dz through float stores (which overflow past float).
double __cdecl Vec3_Distance(const float *a, const float *b);
}
