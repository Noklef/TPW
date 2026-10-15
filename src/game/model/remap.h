// Pointing a model's texture references into a shared texture table: reimplementation of
// 0x46de00.
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
}
