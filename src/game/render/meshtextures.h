// A mesh's textures told to the texture cache: reimplementation of 0x57a420. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// (Test seam: the fncmp driver's table replaces the calls by address.)
void Mesh_SetTextureCalls(const AddrCall *table, u32 count);

// 0x57a420 (stdcall (mesh), pops 4): the level is the float at +4 of [0x879a20] when over 0, else 0. For each of the word +0x5a's records (0x10 bytes at +0x6c), whose pointer is set and has a texture set at its +4: the set's texture 0 or, with bit 0x40 of [0x8bd550], the word +2's (TexSet_Texture).
// With a texture: Texture_SetKeep unless the mesh's +0 has 0xc0000. Word +0 with 0x80 and 0x400: TexCache_UnlinkItem on [0x8c4b44] and 0x400 cleared. TexCache_TouchItem(texture, level). With 0x2000000 in +0, the texture 0 of the set at +4 of +0x98 (each may be null) is touched too, null or not.
extern "C" void __stdcall Mesh_TouchTextures(const u8 *mesh);
