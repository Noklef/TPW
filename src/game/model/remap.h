// Pointing a model's texture references into a shared texture table: reimplementation of
// 0x46de00 and 0x46e130.
//
// The model's faces (+0x44 meshes of 0xa0 bytes at +0x70; a mesh has +0x5a faces of 0x10 bytes
// at +0x6c, each starting with a pointer into the model's own 8-byte texture entries at +0x50)
// and, for a terrain model, its attribute grid's cells (+0x6c: +6 cells, a dword each from
// +0x2c, the texture as u16 at +2) are renumbered through remap, indexed by the model's own
// texture number: v > 0 means first + v - 1, otherwise -1 - v.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46de00: faces get textures + 8 * the new number; grid cells the new number. (The one
// caller, 0x46e4ae, pushes a fifth argument that is never read.)
void __cdecl Model_RemapTextures(u8 *model, const s32 *remap, u32 textures, s32 first);

// 0x46e130 (name unconfirmed; its one caller loads the terrain with the sets 0x74c8a8 and the level's
// "Terrain" directory): gives the model (+0x36 u16 textures; +0x54 of 0x10 bytes, the name at +0xc;
// +0x50 their 8-byte entries) the tiles of sets ({+0 count, +4 groups of 12 bytes: +0 name, +4 tile
// count, +8 tiles of 12 bytes, +4 name, +8 u16 texture}; +8 names to make room for, +0x14 textures
// to make room for). Each model texture whose name a tile has (any case) becomes that tile; the
// others are kept, first, in one new zeroed GlobalAlloc block (sets +0x18; +0x1c / +0x24 the old
// arrays, +0x20 the new entries, +0x28 how many were kept) with room for +0x14 more textures and
// each group's directory (dir then its name). Then the tiles: a texture with flag 1 (+0xa), the
// group's directory (+4) and the tile's name (0x13 characters, in a second block of 0x14 bytes
// each, sets +0x2c), an entry of 1, 0; their numbers (on from the kept ones) into the tiles' +8.
// Sic: when the last texture was found, only the first group's tiles are added (and a model with no
// textures leaves that to the original's stack). Model_RemapTextures then points the faces and the
// grid at the new table, which the model takes (its count the kept ones plus +0x14). Returns 1.
u32 __cdecl Model_AddTerrainSets(u8 *model, u8 *sets, const char *dir);
}
