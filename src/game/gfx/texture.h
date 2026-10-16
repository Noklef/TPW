// The engine textures' small methods: reimplementation of 0x590cb0, 0x590cf0 and 0x590f80-0x590fb0.
//
// A texture keeps its flags at +8 (0x30: the level given in the low 3 bits; 0x1000-0x4000 cache
// state, 0x4000 marking its cache item to be kept, 0x58f83d; 0x30000 bits only the Direct3D
// driver honours) and its width and height (s16) at +0x14 / +0x16. +0x44 is the mask for
// wrapping texture coordinates, set after the size changes (0x5904cd).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x590cb0 (thiscall): +0x44 = (height - 1) << 8 | (width - 1) when both are powers of two above
// 0, else -1.
void __fastcall Texture_UpdateWrapMask(u8 *texture);

// 0x590cf0 (thiscall): the level to use: the low 3 flag bits when 0x30 are both set, else by the
// width: above 128 0, above 64 1, above 32 2, above 16 3, above 8 4, else 5.
u32 __fastcall Texture_Level(const u8 *texture);

// 0x590f80 (thiscall): flags 0x1000, 0x2000 and 0x4000 cleared.
void __fastcall Texture_ClearCacheFlags(u8 *texture);

// 0x590f90 / 0x590fa0 (thiscall): flag 0x4000 (keep its cache item) set / cleared.
void __fastcall Texture_SetKeep(u8 *texture);
void __fastcall Texture_ClearKeep(u8 *texture);

// 0x590fb0 (thiscall): 1 when a flag of 0x30000 is set and the display driver kind (0x8bd524,
// 1 for Direct3D, 0x44de0b) is 1, else 0.
u32 __fastcall Texture_UsesDriverFilter(const u8 *texture);
}
