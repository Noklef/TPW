// The procedural terrain textures' tile cache: reimplementation of 0x5720d0 and 0x572350.
//
// TerrainTex_Init (0x56e550) allocates it (0x10014 bytes, kept at the terrain's +0x230) and reads
// the .lnd into it (0x5723f0); TerrainTexCache_Update (0x572440) is an LRU of tiles keyed by
// (x, y). +4 is 0x40 (its tile count), +8 0x10000 bytes of tables, +0x10010 the tiles in use (the
// destructor 0x572350 frees that many).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5720d0 (thiscall): +4 = 0x40, +0x10008 and +0x10010 = 0, +8..+0x10008 cleared, +0 = 0;
// +0x1000c is left as it was. Returns the cache.
u8 *__fastcall TerrainTexCache_Ctor(u8 *cache);

// 0x572350 (thiscall; name unconfirmed): the destructor. Bit 4 of the dword at 0x8bd550 is cleared; with tiles in use (+0x10010, not 0)
// the texture array at +0xc of each (0x10 bytes from +0x10008; the count signed, re-read) is destroyed when set (Texture_DestroyArray,
// gfx/texture; the pointer kept), the tiles freed and the count cleared; the landscape at +0 (when set) destroyed (Lnd_Dtor), freed and
// cleared. Returns 1.
u32 __fastcall TerrainTexCache_Dtor(u8 *cache);
}
