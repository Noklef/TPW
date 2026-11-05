// A model's height map: its floor rasterised into a byte grid of heights (HeightMap_Build 0x451880 sets the grid up).
#pragma once

#include "../core/addrcalls.h"
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

// HeightMap_Build's calls that the bench replaces go by the game's addresses; a table of replacements (tests/fncmp/heightmapbuild.py gives every one a recorder).
void HeightMap_SetCalls(const AddrCall *table, u32 count);

// 0x451880 (cdecl (instance, width, height, flags, useScale); name invented, unconfirmed): an instance's height grid. A clone of the instance (Instance_Clone(instance, 0, 0x211f, &clone, 0); none: returns 0) reset, a header
// (0x30 bytes, 0xab1e0003, 0x640005, u16 width and height) and a block of 27 bytes per grid square (+0xc 25 heights each; +0x10, +0x14 a byte each). With blocks (+0xa8) and a track on any of the clone's bank slots 2-11 (+0x1c on, 0xc apart): the bounds (+0x18) empty and every block
// not on the slots 0 and 1 played from its start, 30 ms a step, and rasterized at each step until its time left is below 0x6fe3dc (or a NaN). Else the bounds the instance's model's +0x80 and the clone rasterized once. The clone disposed. Then each square's top height (+0x10; with
// useScale the model's float +0x90 * 2.55 instead) and, with flags, 0 at +0x14 when flags[1 + x + 20 * y] (8 bytes each) is 0, else 1. Returns the header.
u8 *__cdecl HeightMap_Build(u8 *instance, u32 width, u32 height, const u8 *flags, u32 useScale);

// 0x451640 (cdecl; name invented, unconfirmed): the instance's height map: read whole from the file path (VFS_Open "rb" etc.; magic 0xab1e0003, version 0x640005; its offsets +0xc, +0x10, +0x14 made pointers); a file
// not read or of other magic is freed. A file of other size is kept (never freed, sic). None of the size width x height: HeightMap_Build and, when that makes one, a copy with its three grids (25 n, n and n bytes,
// the pointers as offsets) written to path (CRT_fopen; none opened: nothing). Then the map's six dwords +0x18 copied to [[instance+8]+4]+0x80, and it set at instance +0xcc. A failed Build faults reading +0x18 (sic).
// Debug_Printf left out.
void __cdecl HeightMap_ReadOrBuild(u8 *instance, u32 width, u32 height, const char *path, const u8 *flags, u32 useScale);
}
