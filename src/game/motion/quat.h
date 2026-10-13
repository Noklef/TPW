// Quaternion helpers of the motion system (see curve.h for the float-exactness approach).
#pragma once

#include "../../../re/types.h"

// The game's sine lookup table (gSinTable, 0x7b0968): values[i] = sin(2 pi i / n), and an
// angle a (radians) is looked up as values[round(a * scale) & mask].
struct SinTable {
    const float *values;
    u32 mask;     // n - 1 (0x7b0960)
    float scale;  // n / (2 pi) (0x7b095c)
};

// A key of a node's rotation track (md2 motion, track flag 0x8).
struct RotationKey {
    u16 frame;
    s16 ease;
    float q[4];  // x, y, z, w
};
static_assert(sizeof(RotationKey) == 20, "RotationKey is 20 bytes");

// 0x4708d0 (n = 4096) as the game first runs it: at the x87's default 53-bit precision,
// before Direct3D exists. values must hold n floats. Not hooked: the game builds the table
// again at shutdown, after Direct3D has lowered the precision, and that copy differs.
void SinTable_Build(float *values, u32 n, u32 *mask, float *scale);

// 0x4742a0 with the table given.
void Quat_SlerpWith(const SinTable &sines, const float *a, const float *b, float t, float *out);

// 0x474490 with the globals given: count = gRotationKeyCount (0x7b4988), flags =
// gMotionFlags (0x7b4980); bit 0x2 lerps instead of slerping.
void Motion_RotationAtWith(const SinTable &sines, const RotationKey *keys, u32 count, u32 flags, u32 index,
                           float t, float *m);

extern "C" {
// 0x474120: rotation matrix (3x3, row major) of q = (x, y, z, w), w the scalar part.
// q need not be unit length: it is scaled by 2 / |q|^2.
void __cdecl Quat_ToMatrix(const float *q, float *m);

// 0x4742a0: slerp from a to b through the game's sine table. Nearly equal quaternions
// lerp; nearly opposite ones turn through a quaternion perpendicular to a.
void __cdecl Quat_Slerp(const float *a, const float *b, float t, float *out);

// 0x474490: rotation matrix between keys index and index + 1 (the last key holds).
void __cdecl Motion_RotationAt(const RotationKey *keys, u32 index, float t, float *m);
}
