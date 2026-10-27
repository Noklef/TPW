// The procedural terrain textures' tile cache: reimplementation of 0x5720d0, 0x572110, 0x572350 and 0x572630.
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

// 0x572110 (thiscall (count, size), pops 8; name unconfirmed): the cache set up for count tiles of size (0: the cache's +4). The size is
// 0x10 when the video memory ([0x8bd520], signed) is under 0x3000000 or the texture memory ([0x8c1624]) under 3 MB times the format's
// unit (0x701548 indexed by [0x8bd554]); a size of 0x40 is 0x20 under 0x4000000 or 5 units. Nothing more with the renderer kind
// [0x8bd524] 4 (returns 1). Else the landscape (Mem_Alloc 0x2ac, Lnd_Ctor) at +0 (none: the error (2, 0xb0754000) pushed onto
// 0x879ad0 and 0), +8..+0x10008 cleared, the tiles (Mem_Alloc count x 0x10, zeroed) at +0x10008, the count at +0x10010 and the size
// at +4; each tile linked in a ring (+0 the one before, +4 the one after) and given a texture (a block of 100: a count of 1, one texture
// built by CRT_VectorVbaseCtorIterator with Texture_Ctor) with Texture_Alloc(size, 0) at +0xc. A failed block or texture: the error
// pushed, TerrainTexCache_Dtor and 0. Last +0x1000c = the middle tile (count >> 1, signed). Returns 1.
u32 __fastcall TerrainTexCache_Init(u8 *cache, void *, s32 count, u32 size);

// 0x572630 (thiscall (texture, x, y, overlay), pops 0x10; name unconfirmed): a tile blended into a texture. Texture_Alloc(texture, the
// cache's size +4, 0); not got: 0. With the format [0x8bd554] of 2-byte pixels (0x701548) the landscape at +0 blends the tile (x, y)
// with the overlay (TerrainBlend_Tile) into the dword at 0x879a68, put into the texture's pixels (Texture_Lock) by
// TexFormat_ConvertSquare at that size (0x879a68 read again after the blend); else into the pixels themselves, each of the size squared
// (signed; re-read each turn) then byte-reversed. Returns 1.
u32 __fastcall TerrainTexCache_BlendTile(u8 *cache, void *, u8 *texture, u32 x, u32 y, const TerrainOverlay *overlay);
}
