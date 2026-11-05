// A level's loaded things freed: reimplementation of 0x457d50. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct AddrCall;
void LevelFree_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x457d50 (cdecl (keep, a byte)): the dynamic faces, world vertices, speech text, model slots 1 and 0, 0x454540, coaster links and types, the
// main light; without keep the model's vertex scratch; all pieces; without keep the sprites; every ride set in [0x7a4614] (the counts
// [0x7a3ebc] + [0x7a4084], reread each time) destroyed by id (its index + 1); without keep the terrain, instance types and highlight buffers;
// the texture folders reset.
void __cdecl Level_FreeAll(u32 keep);
}
