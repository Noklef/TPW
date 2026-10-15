// The terrain attribute grid (TerrainAttrMap_Load 0x450900; the map 0x7a0854 +0x6c): width
// +0x18, height +0x1c, a dword per cell from +0x2c, and a cell count at +6 (u16) for copies.
// Reimplementation of 0x450420, 0x450a30, 0x450b00 and 0x46df50.
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

// 0x46df50: paints cell (x, y) of map's grid with tile of set (from 1; 0 means 1) in sets
// ({count, entries}; an entry is 12 bytes, +4 its tile count, +8 its tiles, 12 bytes each with
// the texture as u16 at +8): the texture into the cell's +2, flags 0x180 set and bits 0 and
// 3-6 cleared, then 0x20 / 0x10 / 8 for a rotation of 90 / 180 / 270. Set and tile wrap
// round their counts (signed). Set 0 with tile 8 erases (|= 0x101, &= ~0x80); with tile 0x37
// restores the cell from the loaded grid (0x79fc54), less bits 3-6 while procedural textures
// are in use. Nothing off the grid. Sic: the set's tile count is first checked (> 0) for the
// set before it wraps.
void __cdecl Terrain_PaintCell(const u8 *map, const u8 *sets, s32 x, s32 y, s32 set, s32 tile, u32 rotation);
}
