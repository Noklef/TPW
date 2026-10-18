// Geometry helpers: reimplementation of 0x5812f0 and 0x581320 (cdecl, x87 at the game's 24-bit precision; used by the mesh builder
// 0x580530), 0x57b410 (a box grown to take a point) and 0x56d520 / 0x56d590 (a segment's point at a given x / z, for the clipping
// at 0x56c7a1-0x56cfde). The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5812f0 (ax, ay, az, bx, by, bz by value): the dot product (bx * ax + by * ay) + bz * az, returned in st0.
double __cdecl Vec3_DotByValue(float ax, float ay, float az, float bx, float by, float bz);

// 0x581320 (a, b, c: points with x at +0, y at +4 and w at +0xc): their x, y and w copied to 0x8c63bc/c8/b0, 0x8c63c0/cc/b4 and
// 0x8c63c4/d0/b8; then with f = (wa * yb - wb * ya) * xc - (wc * yb) * xa and e = (wa * yc - wc * ya) * xb - (wb * yc) * xa, 1 when
// f < e or either is a NaN, else 0: the triangle's winding in homogeneous screen coordinates.
u32 __cdecl Tri_WindingTest(const float *a, const float *b, const float *c);

// 0x57b410 (thiscall on a box: its minimum at +0 and maximum at +0xc, 3 floats each; x, y, z; pops 0xc): each coordinate copied (as a
// dword) to the minimum when below it or unordered (a NaN goes in), and to the maximum when above it.
void __fastcall Box_Extend(u32 *box, void *, u32 x, u32 y, u32 z);

// 0x56d520 (out; a and b, 3 floats each, by value; x): the point of the line through b and a at x, into out, which is returned (a
// struct returned through its hidden pointer). With t = (x - bx) / (ax - bx): (x, (ay - by) * t + by, (az - bz) * t + bz), x87 at the
// game's precision; x goes through the x87 (a signalling NaN comes out quiet).
float *__cdecl Segment_AtX(float *out, float ax, float ay, float az, float bx, float by, float bz, float x);

// 0x56d590: the same at z: with t = (z - bz) / (az - bz), ((ax - bx) * t + bx, (ay - by) * t + by, z), z copied as a dword.
float *__cdecl Segment_AtZ(float *out, float ax, float ay, float az, float bx, float by, float bz, u32 z);
}
