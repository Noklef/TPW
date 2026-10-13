// Quaternion helpers of the motion system (see curve.h for the float-exactness approach).
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x474120: rotation matrix (3x3, row major) of q = (x, y, z, w), w the scalar part.
// q need not be unit length: it is scaled by 2 / |q|^2.
void __cdecl Quat_ToMatrix(const float *q, float *m);
}
