// A set of points drawn as triangles facing the camera.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/pointsprites.py gives them recorders).
void PointSprites_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x58ac90 (thiscall; name invented, unconfirmed): count +0, texture +8 (read first: its null branch further on never runs), positions +0xc (12 bytes each), size +0x10, a matrix +0x18 multiplied (slot 6 of [0x87b1d8], cdecl) with the view (0x87b088) into a static (0xfb3400, an identity at its first use) or the view alone.
// A batch (Batch_Begin: 2, 4, 0x1c4, the texture's format bits, its +0x1c with flag 0x1000); none: 1. Each point in front of the camera gives a triangle: top centre, lower right, lower left, the width the size times 0x8bcbcc, white, the specular left as the stack has it (sic), texture rectangle from the texture's +0x28, +0x2c with +0x20, +0x24 added. A failing add: 0. Else Batch_Close's result.
u32 __fastcall PointSprites_Draw(const u8 *self);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gPointsprites_fb33e0, 0x00fb33e0, 0x1);
