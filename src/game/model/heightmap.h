// A model's height map: its floor rasterised into a byte grid of heights (HeightMap_Build 0x451880 sets the grid up).
#pragma once

#include "../../../re/types.h"
#include "partstate.h"

extern "C" {

// 0x451e20 (cdecl; node, unused, a matrix by value, grid, hit, unused, unused, clipped): the matrix becomes the node's (+0x10)
// times it (Matrix_PreMul). Unless the node's flags have 0x8210: each vertex (u16 count +0x58; blocks of four at +0x60, x, y
// and z each four floats) goes through the matrix (Matrix_TransformPoint) and widens the grid's box (+0x18..+0x2c: least x, y,
// z, then greatest; a NaN counts as least), then is clamped and written back over the node's vertex (sic): x and z into
// [0, the grid's width 0x79fb20 / depth 0x79fb08 - 0.001] (0 below 0; below -1 or past the width / depth + 1 the node goes to
// *hit (when given), *clipped is set and the result is 0), y into [0, 99.999] (*clipped set at 100 or more). Then each face
// (groups: u16 count +0x5a, 16-byte records at +0x6c, first face u16 +4, count u16 +6; 8-byte faces at +0x70, vertices u16 +2,
// +4, +6 through the u16 remap at +0x94) is drawn: its corners as cells (__ftol: x / 2, y * 2.55, z / 2), each edge walked
// row by row (z) into the row table at 0x79f7e8 (16 bytes a row from the face's least z: least x and its height, greatest x
// and its height; reset to 1000000 / -1000000), then each row's span filled into the grid (+0xc, rows of 0x79f7d4 bytes)
// where higher, the heights between the ends by integer steps. Last each child (+0xc, then +8 on) the same with the new
// matrix; 1 when no vertex of the tree went past the grid, else 0.
u32 __cdecl HeightMap_Rasterize(u8 *node, u32 unused, Matrix44 m, u8 *grid, u8 **hit, u32 unused2, u32 unused3, u32 *clipped);
}
