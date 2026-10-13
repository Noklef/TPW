// Quaternion helpers of the motion system (see curve.h for the float-exactness approach).
#pragma once

#include "../../../re/types.h"
#include "../math/sintable.h"

// A key of a node's rotation track (md2 motion, track flag 0x8).
struct RotationKey {
    u16 frame;
    s16 ease;
    float q[4];  // x, y, z, w
};
static_assert(sizeof(RotationKey) == 20, "RotationKey is 20 bytes");

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
