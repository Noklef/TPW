// What a level sets up once its terrain is loaded.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

void LevelSetup_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x457c30 (name invented, unconfirmed): the map's rectangle from the object at [[0x7cf6ec]] into globals, the world size from the grid's cell
// size, the texture rectangle when bit 0x2000 of 0x7a1a8c is set, then the level's buffers and tables. Returns 1.
u32 __cdecl Level_SetupAfterTerrain();

}
