// The terrain attribute grid (TerrainAttrMap_Load 0x450900; the map 0x7a0854 +0x6c): width
// +0x18, height +0x1c, a dword per cell from +0x2c, and a cell count at +6 (u16) for copies.
// Reimplementation of 0x450420, 0x450470, 0x450a30, 0x450b00 and 0x46df50; 0x4508b0, a vector
// helper.
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

// 0x4508b0: out = a x b (b2 a1 - a2 b1, b0 a2 - a0 b2, a0 b1 - b0 a1 on the x87), each
// component stored before the next is read (out may be a or b).
void __cdecl Vec3_Cross(const float *a, const float *b, float *out);

// 0x450950: *out = a copy of map's attribute grid (+0x6c) in one zeroed block: its 0x30-byte
// header, then its +4 dwords from +0x28 and its +6 dwords from +0x2c, the copy's pointers
// moved to them; null when there is no map or grid.
void __cdecl Terrain_CloneAttrs(const u8 *map, u8 **out);

// 0x466f90: the origin cell of a width x height footprint at (x, y) turned by angle (90, 180
// or 270; anything else leaves it): 90 steps y on and takes width off it, 180 steps both on
// and takes width / height off, 270 steps x on and takes height off. The stepped x / y are
// written back (sic).
void __cdecl Terrain_RotateFootprint(s32 *x, s32 *y, s32 angle, s32 *outX, s32 *outY, s32 width, s32 height);

// 0x450470: when there is a map (0x7a0854) with flag 2 (+0x30), Terrain_PaintCell(map, the
// sets 0x74c8a8, x, y, paint[0] set, paint[1] tile, paint[2] rotation) and Frame_AddFlags(1).
void __cdecl Terrain_PaintAt(s32 x, s32 y, const s32 *paint);

// 0x55f210 (thiscall destructor-like): the byte at +4 and the dword at +0 cleared; Array3D_Free on the thirteen arrays at +8, +0x18, ..., +0xc8; the blocks of those from +0xc8 down
// to +0x28 freed and their four dwords cleared; then BufferC_FreeAndClear on +0x18 and +8.
void __fastcall TileSet_Clear(u8 *object);

// 0x55f720 (thiscall; size, pops 4): the byte at +0 = size, the byte at +1 = 0 for 0x40, 2 for 0x10, 1 for 0x20 and any other size (sic: which
// also becomes 0x20); the buffer at 0x8798c8 is freed (when there is one) and a new one of byte(+0)^2 * 4 bytes is made.
void __fastcall Lnd_SetTileSize(u8 *object, void *, u32 size);
}
