// The terrain attribute grid (TerrainAttrMap_Load 0x450900; the map 0x7a0854 +0x6c): width
// +0x18, height +0x1c, a dword per cell from +0x2c, and a cell count at +6 (u16) for copies.
// Reimplementation of 0x450420, 0x450a30 and 0x450b00.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x450420: whether cell (x, y) is park land (bit 0 of its first byte clear): 0 off the grid,
// 1 when there is no map or grid.
u32 __cdecl Terrain_IsParkLand(u32 x, u32 y);

// 0x450a30: copies grid's cells (+0x2c) over map's own grid (+0x6c) cells, as many dwords as
// map's grid counts (+6), when map, its grid, both cell arrays and the count are all there.
void __cdecl Terrain_CopyAttrs(const u8 *map, const u8 *grid);

// 0x450b00: the grid's width and height (the map and grid must be there).
void __cdecl Terrain_GridSize(u32 *width, u32 *height);
}
