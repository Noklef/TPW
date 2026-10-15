// Procedural terrain textures on and off: reimplementation of 0x457a00-0x457a87, 0x46ded0 and
// 0x56dfb0 / 0x56dfd0.
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
}
