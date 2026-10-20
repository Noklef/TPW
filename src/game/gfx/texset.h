// The texture sets' constructor, getters, release, set-up and loads: reimplementation of 0x575bd0, 0x575f90, 0x575fa0, 0x575f00,
// 0x575c60, 0x575ee0, 0x575e10 and 0x575b90.
//
// A texture set (16 bytes) is a scene file's list of textures for one material (built at 0x579f45
// into the scene's +0x50 table, filled by 0x575c60; the terrain looks them up, 0x56f212): +8 the
// textures, +0xc how many (s16; 0x8000 when they share one block, freed whole), +0xe how many
// users (0x575f00 frees the textures when it drops to 0). +0 its definition (byte 0: 0x10 / 0x20 the textures' flags 0x10000 /
// 0x20000, 0x30 0x30000 on a set-up again, 0x80 one blank texture; byte 1: 0x10 / 0x20 / 0x40 the texture manager's load flags 2 / 4
// / 8), +4 its file info (+4 the folder, +0xa the count (u16), +0xc the names, 0x14 bytes each; +6 of the first the blank texture's
// size). While the format flags (0x8bd550) have 0x40000000 the loads wait in the list at 0x87a270 (count at 0x87a670, up to 0x100).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x575c00 (thiscall; array; name unconfirmed): all zero, then a list of one (TrackedGlobal_Alloc, core/gameheap) at +8 holding the shared
// texture array, +0xc = 0x8000 (shared) and one user: 1. When the list was not got, the error (2, 0xb0754000) pushed (EngineErrors_Push)
// and 0.
u32 __fastcall TexSet_InitShared(u8 *set, void *, u32 array);

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

// 0x575e10 (thiscall; name unconfirmed): for each of the count's textures (+0xc, signed, re-read) the texture manager ([0x87a268],
// re-read) gets the load flags the definition's byte 1 asks for (2, 4, 8 in turn, so the last counts, sic) and the textures of the
// folder and name i are looked up (TexManager_Acquire) into the list; got, the definition's byte 0 0x10 / 0x20 set flags 0x10000 /
// 0x20000 on them. Returns 1.
u32 __fastcall TexSet_LoadTextures(u8 *set);

// 0x575b90 (cdecl; name unconfirmed): TexSet_LoadTextures on each set waiting at 0x87a270 (the count at 0x87a670, re-read), then
// the count cleared.
void __cdecl TexSet_LoadDeferred();

// 0x575c60 (thiscall; definition, file info; pops 8; name unconfirmed): a set with its list (+8) gets a user more (+0xe) and, with
// the definition's 0x30, flag 0x30000 on each of its textures (+0xc, signed, re-read): 1. Else +0 / +4 the definition and info and
// the list (the info's count * 4, TrackedGlobal_Alloc; not got: 0). With the definition's 0x80 one texture (count-prefixed,
// GameHeap_Alloc and CRT_VectorVbaseCtorIterator on Texture_Ctor; +0xc 0x8000 either way) given pixels (Texture_Alloc with the first
// name's u16 +6 and 1): done, it is the list's first, its flags 0x1000240, one user and 1; else the array destroyed (when got) and the
// error (2, 0xb0754000) pushed (EngineErrors_Push): 0. Otherwise +0xc the info's count, the set loaded now (TexSet_LoadTextures) or
// later (0x8bd550's 0x40000000: added to the list at 0x87a270 below 0x100, else not at all), one user and 1.
u32 __fastcall TexSet_Init(u8 *set, void *, const u8 *def, const u8 *info);

// 0x575ee0 (thiscall; pair; pops 4; name unconfirmed): TexSet_Init with the pair's two dwords.
u32 __fastcall TexSet_InitFromPair(u8 *set, void *, const u32 *pair);
}
