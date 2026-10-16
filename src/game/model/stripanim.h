// The strip animation of an object that grows or shrinks by a factor t (its +0xb8, stepped by
// +0xbc in 0x4547f0 / 0x4548f0): reimplementation of 0x4543c0 and 0x4547c0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4543c0: when the object's flags (+4) have 2, for each of the [+0xb0] strips i (from 0, 4
// vertices each from vertex 2 = 4 i + 2, of the shape at +0xb4: grouped vertices at +0x10, y
// 0x10 into each 0x30-byte group; texture coordinates at shape +0x24 -> +4 -> +0x78 -> +0x68,
// u and v 0x10 apart in 0x20-byte groups):
//   v = 1 - ((i / n * 0.8 - (t - 0.25)) * 4), then 0 when below 0 (or NaN) and 1 when above 1;
//   w = (0.8 for odd i, else 1) * v; the y of vertices j - 2 and j - 1 = that of j and j + 1
//   plus 10 w; the coordinates of j = (0, 1 - w) and of j + 1 = (1, 1 - w).
// Nothing for a count of 0 or less.
void __cdecl StripAnim_Apply(const u8 *object, float t);

// 0x4547c0: when it has strips (+0xb0), the step (+0xbc) = -0.3, and flags 0x20 trade 0x40 for 0x80.
void __cdecl StripAnim_StartShrink(u8 *object);
}
