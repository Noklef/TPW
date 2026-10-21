// The terrain attribute grid (TerrainAttrMap_Load 0x450900; the map 0x7a0854 +0x6c): width
// +0x18, height +0x1c, a dword per cell from +0x2c, and a cell count at +6 (u16) for copies.
// Reimplementation of 0x450420, 0x450470, 0x450a30, 0x450a80, 0x450b00 and 0x46df50; 0x4508b0, a vector
// helper; the ride at a cell, 0x450ac0 and 0x452a60.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x450420: whether cell (x, y) is park land (bit 0 of its first byte clear): 0 off the grid,
// 1 when there is no map or grid.
u32 __cdecl Terrain_IsParkLand(u32 x, u32 y);

// 0x450a30: copies grid's cells (+0x2c) over map's own grid (+0x6c) cells, as many dwords as
// map's grid counts (+6), when map, its grid, both cell arrays and the count are all there.
void __cdecl Terrain_CopyAttrs(const u8 *map, const u8 *grid);

// 0x450a80 (name unconfirmed): the loaded grid (0x79fc54, Terrain_CloneAttrs's) given back (GlobalMem_Free) and the instance at
// 0x79fcb4 freed (Instance_Free, not kept), each when there and then cleared; then GlobalMem_FreeThree_427bb0.
void __cdecl Terrain_FreeLoaded();

// 0x450b00: the grid's width and height (the map and grid must be there).
void __cdecl Terrain_GridSize(u32 *width, u32 *height);

// 0x452a60 (cdecl (x, y); name unconfirmed): the index of the ride at map cell (x, y): the cell's dword +4 (MapCell_Dword4At) when
// not 0; else the object there (Map_ObjectOrEntranceAt's handle; 0 for none) when its thing (0x7cfb90, 20 bytes a slot) is there
// and of kind 3 (the byte +2): its dword +0x20; else 0. (Its debug check of the thing table left out.)
u32 __cdecl Map_RideIndexAt(s32 x, s32 y);

// 0x450ac0 (cdecl (x, y); name unconfirmed): the ride (0x7a4610 by index) at the grid's cell (x, y), each clamped below the grid's
// width / height (unsigned; the map and grid must be there) by Map_RideIndexAt; null for index 0.
u8 *__cdecl Terrain_RideAt(u32 x, u32 y);

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

// 0x55f630 (thiscall): the bytes +2 .. +0xb and the dwords +0xc and +0x10 cleared, Array3D_Free on +0x14 and, for the three levels (0xd8 bytes each from +0x2c), the thirteen arrays at +0 .. +0xc0
// of the level (Array3D_Free), its tile size (the dword at -8: 0x40, 0x20, 0x10) and its number (the byte at -4).
void __fastcall Lnd_Clear(u8 *object);

// 0x55f040 (thiscall; name unconfirmed): a level's constructor: the bytes +8 .. +0xd7 cleared, the byte at +4 and the dword at +0 too (+5 .. +7 kept), then
// Array3D_Free on the thirteen arrays at +8 .. +0xc8. Returns the object.
u8 *__fastcall TileSet_Ctor(u8 *object);

// 0x55f470 (thiscall; stream, pops 4): a level read: a u32 into +0, a byte into +4 (stream slot 8), then Array3D_Read of the arrays at +8, +0x18, +0x38,
// +0x48, +0x88, +0x98, +0x28, +0x58, +0xa8, +0x68, +0x78, +0xb8 and +0xc8 in that order. Returns 1 in AL over the upper bytes of the last read's return.
u32 __fastcall Lnd_ReadLevel(u8 *object, void *, u8 *stream);

// 0x560ff0 (thiscall; name, pops 4; the game pushes a second argument to VFS_Open, 0x74cfb8, unread): base.lnd read. Lnd_Clear, then the
// file opened (VFS_Open; none: 0); its version byte (not 3: the stream finished and deleted, 0); the bytes +2 .. +0xb; the ramps at 0x8794a8
// (centre +0xa, width +0xb) and 0x8793a8 (+6, +7): with w = width + 1, lo = centre - w / 2 and hi = centre + w / 2 (both as s16), entry i
// (0..255) is 0 below lo, 0xff above hi, else (255 i - 255 lo) / w; then Lnd_BuildRamp of 0x8797a8 (+8, +9), 0x8796a8 (+4, +5) and
// 0x8795a8 (+2, +3); the control map Array3D_Read into +0x14, its width - 1 and height - 1 (+0x14, +0x18) over 4 (sar) into +0xc and +0x10.
// Then for each of the three levels (0xd8 bytes each from +0x24): the one byte +1 names read with Lnd_ReadLevel; the others skipped: a u32
// and a byte read and dropped, then thirteen times three dwords read and a seek from here (Stream_SeekFailed, whence 1) by their product.
// The stream finished and deleted; returns 1 (in AL over Object_FinishAndDelete's 0).
u32 __fastcall Lnd_Read(u8 *object, void *, const char *name);

// 0x55f530 (thiscall; name unconfirmed): the dwords +0x14 .. +0x20 cleared, the three levels at +0x24 (0xd8 bytes each) made with TileSet_Ctor, then
// Lnd_Clear. Returns the object.
u8 *__fastcall Lnd_Ctor(u8 *object);

// 0x55f5a0 (thiscall; name unconfirmed): Lnd_Clear; the buffer at 0x8798c8 freed (when there is one) and cleared; the three levels' TileSet_Clear, the last
// first; the block at +0x20 freed when still set and the dwords +0x14 .. +0x20 cleared.
void __fastcall Lnd_Dtor(u8 *object);

// 0x4527f0 (cdecl; x, y): the ground height at (x, y) on the map's (0x7a0854) grid (+0x6c), in st0: 0 for x or y below 0 (or NaN) or
// not below 10 times the grid's width (+0x18) / height (+0x1c); else the cell's column and row the whole parts of x / +0x10 and y / +0x14
// (CRT modf, __ftol), fx and fy their fractions (as floats), and the heights (floats from +0x28, a row of width + 1 to a row of cells)
// at its corners h00, h10 (this row), h01, h11 (the next) blended over one of the cell's triangles: without the cell's flag 0x800 (or with
// flags 0x804 and fx < 1 - fy, or NaN) (h10 fx + (1 - fx) h00) + (h01 - h00) fy; 0x804 otherwise ((1 - fx) h01 + fx h11) - (h11 - h10)
// (1 - fy); 0x800 alone with fx < fy (or NaN) (h11 fx + (1 - fx) h01) - (h01 - h00)(1 - fy), else ((1 - fx) h00 + fx h10) + (h11 - h10) fy;
// on the x87, in that order. The grid is read first, whatever x and y are.
double __cdecl Terrain_HeightAt(float x, float y);

// 0x452560 (cdecl; x, z, any; name unconfirmed): the height at (x, z) on the same grid from its origin (+8, +0xc), in st0: 0 for x or z
// below 0 (or NaN) or not below 10 times the width / height, or for a grid position gx = (x - +8) / +0x10 below 0 (or NaN, before its
// rounding to a float) or above the width, or gz = (z - +0xc) / +0x14 below 0 (or NaN) or above the height. Unless any, FLT_MAX for a
// cell (+0x2c, row width) with flag 1. Else the cell's column ix = __ftol(gx) (with fx its CRT modf fraction, as a float) and row iz (the
// whole part of gz; fz its fraction) and the corners h00, h10, h01, h11 (row width + 1) blended over a triangle: with the cell's flag 4,
// when fx >= 1 - fz ((1 - fx) h01 + h11 fx) + (1 - fz) -(h11 - h10), else ((1 - fx) h00 + h10 fx) + fz (h01 - h00); without, when fx < fz
// (or NaN) ((1 - fx) h01 + h11 fx) + (1 - fz) -(h01 - h00), else ((1 - fx) h00 + h10 fx) + fz (h11 - h10). On the x87, in that order.
double __cdecl Terrain_GridHeightAt(float x, float z, u32 any);

// 0x454050 (cdecl; angle, x0, y0, x1, y1, u, v, &cellX, &cellY): the cell under the point at fractions (u, v) of a footprint from
// (x0, y0) to (x1, y1) turned by angle, each __ftol((t b + (1 - t) a) + 0.1) on the x87: angle 0 cellX from u over x0..x1 and cellY
// from v over y0..y1; 90 cellX from v over x0..x1, cellY from u over y1..y0; 180 u over x1..x0, v over y1..y0; 270 v over x1..x0, u over
// y0..y1. Any other angle writes nothing.
void __cdecl Terrain_FootprintCell(s32 angle, float x0, float y0, float x1, float y1, float u, float v, s32 *cellX, s32 *cellY);

// 0x466d70 (cdecl; x, y, cells, width, depth, flags, angle): the ground height under a width x depth footprint at cell (x, y), in st0:
// (lowest + highest) / 2 of the grid heights (as Terrain_HeightAt reads them, row width + 1) at the corners Terrain_FootprintCell gives
// for each of its cells (the footprint's far corner (x + w, y + d), or (x + d, y + w) unless angle is 0 or 180; fractions stepped by
// 1 / w and 1 / (float)d as floats). A cell counts when cells is null, flags has 0x10, or Flags_AnyLow4 of its dword (cells + 8 + 8 i +
// 0xa0 j). The extremes start at -10000 and 10000; a NaN height becomes the lowest, and stays so. With an angle Terrain_FootprintCell
// does not take the corner read is at the float bits of x and y (sic).
double __cdecl Terrain_FootprintHeight(s32 x, s32 y, const u8 *cells, s32 width, s32 depth, u32 flags, s32 angle);

// 0x452ae0 (cdecl; x, y, slot; name unconfirmed): the height of the surface at cell (x, y), in st0. The ride is *slot when slot and *slot are
// set, else the one at the cell (Map_RideIndexAt inline, its debug check left out). The position is x * 10, y * 10 (fild, as floats). With a
// ride (to *slot when given) whose record (0x7a4610) has a height map (+0xcc): from the origin (+0x40 of the +0x78 of the +4 of the record's
// +8, whose +0x10 is the angle) dx = x - +0x40 (a float), dy = y - +0x48 (kept on the x87), turned (90: u = -dy - 10, v = dx; 180: u =
// -dx - 10, v = -dy - 10; 270: u = dy, v = -dx - 10; any other: u = dx, v = dy), then the byte at the map's +0x10 less its width (u16 +8)
// times __ftol(v -0.1) less __ftol(u -0.1), times 100 / 255 plus the origin's +0x44. Otherwise *slot = 0 and 0 when slot is given, else
// Terrain_HeightAt of the position.
double __cdecl Terrain_SurfaceHeight(s32 x, s32 y, u32 *slot);
}
