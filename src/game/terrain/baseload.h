// Loads the map's terrain model, base.md2, and sets up what depends on its height grid.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

void TerrainBase_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x4504c0: `shared` nonzero takes the folder from [0xf81704], otherwise from GamePaths_Base. Returns 0 when neither "Base" nor
// "TestBase" gives a model. Each grid square whose two triangles' normals differ by more than the threshold gets bit 0x800 in its cell.
// `texture_arg` goes on to TerrainTex_InitGlobal.
u32 __cdecl Terrain_Load(u32 shared, u32 texture_arg);

}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gTerrain_7a0884, 0x007a0884, 0x4);
