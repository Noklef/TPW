// What a level sets up once its terrain is loaded.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

void LevelSetup_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x457a90 (name invented, unconfirmed): the seed 0x7b0958 drawn (rand), the motion flags cleared, the camera reset, the terrain loaded (Terrain_Load) with a texture argument from the detail
// flags 0x79d86c (bit 4: 0x10, bit 2: 0x20, bit 1: 0x40, none: 0x20), both advisors' models loaded (the second with "_" and three characters of the language at [0x786b70]), the advisor
// set up, the main light made and the tints reset. Returns 1. Debug_Printf left out.
u32 __cdecl Level_LoadTerrainAndAdvisor();

// 0x457c30(name invented, unconfirmed): the map's rectangle from the object at [[0x7cf6ec]] into globals, the world size from the grid's cell
// size, the texture rectangle when bit 0x2000 of 0x7a1a8c is set, then the level's buffers and tables. Returns 1.
u32 __cdecl Level_SetupAfterTerrain();

}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gLevelsetup_7b0958, 0x007b0958, 0x4);
