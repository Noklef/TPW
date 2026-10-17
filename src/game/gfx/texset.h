// The texture sets' constructor, getters and release: reimplementation of 0x575bd0, 0x575f90, 0x575fa0 and 0x575f00.
//
// A texture set (16 bytes) is a scene file's list of textures for one material (built at 0x579f45
// into the scene's +0x50 table, filled by 0x575c60; the terrain looks them up, 0x56f212): +8 the
// textures, +0xc how many (s16; 0x8000 when they share one block, freed whole), +0xe how many
// users (0x575f00 frees the textures when it drops to 0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x575bd0 (thiscall): all zero; returns the set.
u8 *__fastcall TexSet_Ctor(u8 *set);

// 0x575f90 (thiscall; i): texture i.
u32 __fastcall TexSet_Texture(const u8 *set, void *, s32 i);

// 0x575fa0 (thiscall; i): where texture i is kept.
u32 *__fastcall TexSet_TextureSlot(const u8 *set, void *, s32 i);

// 0x575f00 (thiscall; name unconfirmed): one user less (+0xe); when that leaves none, the textures: with flag 0x8000 the shared array
// (the list's first) released (Texture_Release) and destroyed when set (Texture_DestroyArray), else each of the count's (+0xc, signed,
// re-read) released from the texture manager at [0x87a268] (TexManager_ReleaseTexture); then the list freed (TrackedGlobal_Free) and
// cleared, 1. Users left: 0.
u32 __fastcall TexSet_Release(u8 *set);
}
