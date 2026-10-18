// Procedural terrain textures on and off: reimplementation of 0x457a00-0x457a87, 0x457e10-0x457e50,
// 0x46ded0 and 0x56dfb0 / 0x56dfd0.
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
}
