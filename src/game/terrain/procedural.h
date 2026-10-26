// Procedural terrain textures on and off: reimplementation of 0x457a00-0x457a87, 0x457e10-0x457e50,
// 0x46ded0 and 0x56dfb0 / 0x56dfd0, and the column extents 0x56e780 its texture's rasterising (0x56e7e0-) widens.
//
// 0x7a1a8c bit 0x2000 says the procedural textures could be loaded (TerrainTex_Load 0x56dfe0),
// 0x4000 that they are in use; the renderer's flags 0x8bd550 have them at bit 2 (unless bit
// 0x10000000 disables them). While they are in use the terrain cells (the map 0x7a0854 +0x6c
// grid, a dword per cell from +0x2c, u16 flags first) lose their texture rotation bits 3-6;
// turning them off copies bits 3-7 back from the grid as loaded (0x79fc54). Cells with 0x81
// set are left alone either way.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x56dfb0: renderer flags 0x8bd550 |= 4, unless they have 0x10000000.
void __cdecl TerrainTex_Enable();

// 0x56dfd0: renderer flags 0x8bd550 &= ~4.
void __cdecl TerrainTex_Disable();

// 0x56df70 (x0, y0, x1, y1): when the procedural texture object [0x879d60] exists, its +0x234 to
// +0x240 = the four (0x457cf5: the terrain area in view).
void __cdecl TerrainTex_SetRect(u32 x0, u32 y0, u32 x1, u32 y1);

// 0x56ddf0 (name unconfirmed): when there is no procedural texture object yet, a new one of 0x12974 bytes (the game's heap) at [0x879d60], its
// +0, +4, +0x20, +0x24, +0x128, +0x12c, +0x230 to +0x238, +0x2958, +0x12968 and +0x12970 cleared and the rectangle's right / bottom (+0x23c /
// +0x240) 0x7f; 1 then. 0 when there was one, or when the allocation failed (left null, the error (3, 0xb0754000) pushed onto the engine's list).
u32 __cdecl TerrainTex_Create();

// 0x56e4e0 (thiscall; name unconfirmed): the procedural texture object's tile cache (+0x230, when set) destroyed (TerrainTexCache_Dtor,
// terrain/texcache), then, re-read, destroyed again through its jump 0x572100 and freed (the game's heap), and cleared; the tracked block
// at +0x12970 (when set) freed (TrackedGlobal_Free) and cleared; +0 and +4 = 0. Returns 1. The terrain's unload (0x57a23b) calls it on
// [0x879d60], which is kept.
u32 __fastcall TerrainTex_Unload(u8 *tex);

// 0x56de90 (cdecl; name unconfirmed): when the procedural texture object [0x879d60] exists, it is unloaded (TerrainTex_Unload, inlined), then,
// re-read, unloaded again (sic) and freed (the game's heap); [0x879d60] = 0. Returns 1.
u32 __cdecl TerrainTex_Destroy();

// 0x56e030 (thiscall (x0, y0, x1, y1, delta); pops 0x14; name unconfirmed): delta added to each float of the grid at [+0x12970] (rows of
// width + 1, the width and height the dwords +0x18 / +0x1c of the header [+0]) for rows y0..y1 and columns x0..x1 (signed, inclusive);
// negative ones and those past the width / height (unsigned) are left. The header and the grid pointer are read for each cell.
void __fastcall TerrainTex_AddRect(u8 *tex, void *, s32 x0, s32 y0, s32 x1, s32 y1, float delta);

// 0x56e0a0 (thiscall; name unconfirmed): fixed raises of the grid (TerrainTex_AddRect's, clipped the same way): 4.0 over the six
// rectangles (x0, y0)-(x1, y1) (0x1c, 0)-(0x1e, 0xc), (0x1f, 0xb)-(0x2b, 0xd), (0x2c, 0xb)-(0x2e, 0x10), (0x1d, 0)-(0x1d, 0xc), (0x1e, 0xb)-
// (0x2c, 0xc) and (0x2d, 0xb)-(0x2d, 0x10) (inlined there as cell - -4.0, the same sums), then 21 TerrainTex_AddRect calls: six of 1.0
// about columns 0x1f-0x2f, six of 4.0 about 0x32-0x44, six of 1.0 about 0x30-0x41 and three of 1.4 over rows 0-6.
void __fastcall TerrainTex_StampFixed(u8 *tex);

// 0x56e3f0 (thiscall (land), pops 4; name unconfirmed; the terrain's load 0x579e00 asks it): the header [land +0x6c] at +0, the land's +0x50
// at +4; +8 / +0xc = the header's width / height (+0x18 / +0x1c) + 1; +0x10 / +0x14 = 1.0 / the header's floats +0x10 / +0x14; +0x18 =
// 1.0 / (+0x24 - +0x20) of the header, 0.0625 when that is 0 or a NaN (x87, the game's precision); +0x12968 = 0. With the render flag
// 0x10000 (0x8bcbc8): the tracked block +0x12970 (when set) freed and cleared, a new one of the header's word +4 times 4 bytes
// (TrackedGlobal_Alloc) put there and, when got, filled from the header's +0x28 and stamped (TerrainTex_StampFixed). Then
// TerrainTex_SetView, whose 1 is returned.
u32 __fastcall TerrainTex_Setup(u8 *tex, void *, const u8 *land);

// 0x56e6c0 (thiscall; name unconfirmed): the world view [0x879d54] set from the header [+0]: its x range from the float +8 to that plus
// the width (+0x18, unsigned) times the float +0x10, in those steps, the y range likewise from +0xc, +0x1c and +0x14 (WorldView_SetRangeX /
// Y; x87, the product rounded once), +0x30 / +0x34 = the header's +0x20 / +0x24 (Obj_Set30_34) and its diagonal (WorldView_UpdateDiagonal);
// then the renderer's [0x87a6d0] frame buffer made at the width by the height (Renderer_MakeFrameBuffer). Returns 1.
u32 __fastcall TerrainTex_SetView(u8 *tex);

// 0x56ef10 (thiscall; name unconfirmed): the grid's colours lit, once a frame: nothing when the stamp +0x12968 is the frame number
// 0x8bd50c already and 0x8bd508's 0x40000000 is clear. Else for each of the +0xc rows (signed) a row of the +8 points (signed; at most 128,
// the stack's room) from the header [+0]'s height floats [+0x28] on (a row after the last's points, rows +8 apart): each point x, the
// height, z, x starting at the header's float +8 and stepping by it (sic: not +0x10), z from the header's +0xc stepping by its +0x14 a
// row; its normal (dx, 1.0, dz) the height differences scaled by +0x18: dx the left less the right neighbour (the point itself at an
// end), dz the row before less the row after (the point itself in the first row; in the last, the height 128 floats back (sic) less
// the point), each stored as a float before the scaling (x87 at the game's precision); the row lit by Meshes_LightPoints on [0x87a248]
// into +0x2968 + 0x200 a row. Then the stamp = the frame number.
void __fastcall TerrainTex_LightGrid(u8 *tex);

// 0x457a00: when available and not in use: the option 0x78d8d8 at 1 turns them on (flag, the
// renderer, the cells); at 0 marks them unavailable (clears 0x6000); else nothing.
void __cdecl Terrain_ProceduralEnter();

// 0x457a40: when in use, turns them off (flag, the renderer, the cells).
void __cdecl Terrain_ProceduralLeave();

// 0x457a70: 1 when available and in use.
u32 __cdecl Terrain_ProceduralOn();

// 0x46ded0: on: clears bits 3-6 of each cell; off: replaces bits 3-7 with the loaded grid's.
// Reads the loaded grid's cell pointer either way.
void __cdecl Terrain_SetProceduralCells(u32 on);

// 0x457e10: every cell back as loaded (Terrain_CopyAttrs(*0x7a0854, *0x79fc54)); the save
// (0x41501d) does it unless saving the slot-1 kind.
void __cdecl Terrain_RestoreCells();

// 0x457e30, 0x457e50: the unit's unused statics, 0x7a0b28 = 1.0f and 0x7a0b48 = 1 / 2^23.
void __cdecl Terrain_InitUnusedOne();
void __cdecl Terrain_InitUnusedEpsilon();

// 0x56e780 (thiscall; low, first, high, last, pops 16): the extents of a column range. +0x20 =
// min(+0x20, first) and +0x24 = max(+0x24, last) (signed); then for each column from first to last
// inclusive, the signed byte at +0x28 + column is lowered to the low byte of `low` when above it
// (compared as a dword) and the byte at +0xa8 + column raised to the low byte of `high` when below.
void __fastcall Extents_Include(u8 *extents, void *, s32 low, s32 first, s32 high, s32 last);
}
