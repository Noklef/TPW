// Two geometry helpers of the mesh builder (0x580530): reimplementation of 0x5812f0 and 0x581320 (cdecl, x87 at the game's 24-bit
// precision). The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5812f0 (ax, ay, az, bx, by, bz by value): the dot product (bx * ax + by * ay) + bz * az, returned in st0.
double __cdecl Vec3_DotByValue(float ax, float ay, float az, float bx, float by, float bz);

// 0x581320 (a, b, c: points with x at +0, y at +4 and w at +0xc): their x, y and w copied to 0x8c63bc/c8/b0, 0x8c63c0/cc/b4 and
// 0x8c63c4/d0/b8; then with f = (wa * yb - wb * ya) * xc - (wc * yb) * xa and e = (wa * yc - wc * ya) * xb - (wb * yc) * xa, 1 when
// f < e or either is a NaN, else 0: the triangle's winding in homogeneous screen coordinates.
u32 __cdecl Tri_WindingTest(const float *a, const float *b, const float *c);
}
