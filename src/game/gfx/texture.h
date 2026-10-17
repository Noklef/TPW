// The engine textures' small methods: reimplementation of 0x590a00, 0x590b90, 0x590c40, 0x590cb0, 0x590cf0
// and 0x590f80-0x590fb0.
//
// A texture keeps its flags at +8 (0x30: the level given in the low 3 bits; 0x1000-0x4000 cache
// state, 0x4000 marking its cache item to be kept, 0x58f83d; 0x30000 bits only the Direct3D
// driver honours) and its width and height (s16) at +0x14 / +0x16. +0x44 is the mask for
// wrapping texture coordinates, set after the size changes (0x5904cd). The texture class has a
// virtual base at +0x48 (its vbtable 0x702538 holds 0, 0x48).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x590a00 (thiscall): the constructor. When most derived, the vbtable pointer and the virtual
// base's six dwords (+0x48-+0x5c) first; then +0xc = -1 and the other fields to +0x40 zeroed.
// Returns the texture.
u8 *__fastcall Texture_Ctor(u8 *texture, void *, u32 mostDerived);

// 0x590c40 (thiscall): the format word for the driver: 0x400000 when it uses the driver filter
// (as 0x590fb0), 0x7000000 when flag 0x4000000 is set, and by flags & 0x300: 0x100 0x2756,
// 0x200 0x556, 0x300 0x200552, none 0x1256.
u32 __fastcall Texture_FormatBits(const u8 *texture);

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

// 0x590ff0 (thiscall; name unconfirmed): the counter at 0x8c4b60 is raised, the dword at +0x14 takes the device generation (0x8bcc40) and the object at +0 answers with its virtual call 5,
// whose result is returned.
u32 __fastcall Texture_Reload(u8 *texture);

// 0x590b90 (thiscall; name unconfirmed): the release (the texture arrays' destructor, through 0x590a70). With flag 0x800000 the texture
// after it (+0x60) first; with flag 0x1000 its cache item is unlinked (TexCache_UnlinkItem on the cache at 0x8c4b44); with flag 0x10000000
// the object at +0x40 (when set) is deleted (its virtual call 0 with 1; +0x40 left as it is), else the pixels at +0x40 are freed
// (TrackedGlobal_Free) and cleared; so is the block at +0x3c. Then the sizes at +0x10-+0x16 are zeroed, the flags 5 and +0x38 the cache key's
// level part alone (<< 16). Returns 1.
u32 __fastcall Texture_Release(u8 *texture);
}
