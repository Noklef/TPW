// Arrays of vertices transformed by a 4x4 matrix: reimplementation of 0x579250 and 0x579330 (cdecl, x87 at the game's 24-bit
// precision; reached through the virtual-base thunks 0x579070 and 0x579060). The names are unconfirmed.
//
// The matrix is 16 floats, rows of 4: a vertex (x, y, z, w) comes out as x * row 0 + y * row 1 + z * row 2 + w * row 3, with w = 1
// for a point. Each vertex's inputs are all read before any of its outputs is stored, so the output may be the input.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x579250 (self, unused; matrix, out, in, out stride in bytes, first, count): points kept in blocks of four, 0x30 bytes each (x[4],
// y[4], z[4]); count of them from the first, each to x, y, z, w floats at out (w at +0xc, stored first, then y, x and z), out
// stepping by the stride. Only bits 0-2 of first are used: the start is block (first & 4 ? 1 : 0), slot first & 3. A count of 0
// does nothing; the count is unsigned.
void __cdecl Transform_BlockedPoints(void *self, const float *matrix, float *out, const u8 *in, s32 out_stride, u32 first, u32 count);

// 0x579330 (self, unused; matrix, out, in, out stride, in stride (bytes), count, mode): count vertices, each read from in and
// written to out, the two stepping by their strides. mode 1: x, y, z, w in, 4 out; 2: a point (x, y, z) in, x, y, z out; 3: a
// direction (no translation), 3 out; any other: a point in, x, y, z, w out.
void __cdecl Transform_Vertices(void *self, const float *matrix, float *out, const u8 *in, s32 out_stride, s32 in_stride, u32 count,
                                s32 mode);
}
