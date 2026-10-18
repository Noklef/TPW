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

// 0x590a80 (thiscall; size, flags; pops 8; name unconfirmed): the pixels, a tracked block (TrackedGlobal_Alloc, core/gameheap) of the
// format's pixel size (0x7024e0 indexed by [0x8bd554]) times size squared, at +0x40; not got: the error (2, 0xb0754000) pushed
// (EngineErrors_Push) and 0. Got: the words +0x10 to +0x16 and +0x34 = size, +8 = flags << 6 | 0x1000000 | the level (0 above 0x80, 1
// above 0x40, 2 above 0x20, 3 above 0x10, else 4, 5 from 8 down; signed), +0x38 = (__ftol((1.5 - the float at +0x30) * 2047) & 0xfff)
// << 16 plus the s16 size * 0x101, with 0x20000000 for flag 0x4000 and 0x40000000 for 0x80000. Returns 1.
u32 __fastcall Texture_Alloc(u8 *texture, void *, u32 size, u32 flags);

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

// 0x590eb0 (thiscall; width, height, pops 8; name unconfirmed): the u16 width at +0x14 and +0x10 and the height at +0x16 and +0x12; the
// flags' low 3 bits the size class of the greater (s16) side: above 0x80 0, 0x40 1, 0x20 2, 0x10 3, else 4, 5 for 8 or less; then the
// cache key as TexCacheItem_SetKey (gfx/texcache) sets it (inlined).
void __fastcall Texture_SetSize(u8 *texture, void *, u32 width, u32 height);

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

// 0x590d40 (thiscall; name unconfirmed): the pixels. Without flag 0x10000000 the pointer at +0x40; with it the object at +0x40 is
// locked (its virtual call 2 with a stack record: the dword at 0x879a68, the s16 sizes at +0x10 and +0x12 widened, the format's pixel
// size (0x7024e0 indexed by [0x8bd554]) times the first) and the dword at 0x879a68 is returned as read after it.
u8 *__fastcall Texture_Lock(u8 *texture);

// 0x590da0 (thiscall; name unconfirmed): Texture_Lock's pixels, turned upside down first with flag 0x40000: for each row below the s16
// at +0x12 halved, its dwords (the pitch, pixel size times the s16 at +0x10, over 4) swapped with the row the s16 at +0x10 counts from
// the end (sic: the width, not the height); nothing when the s16 at +0x12 with bit 0 cleared is not above 0. A pitch under 4 wraps
// the dword count (sic). Returns the pixels.
u8 *__fastcall Texture_LockFlipped(u8 *texture);
}

// A count-prefixed array of textures (the count in the dword before it; not 0) destroyed, Texture_Release on each from the last
// (CRT_VectorDtorIterator: the originals pass Texture_Release's jump at 0x590a70), then freed 4 bytes below (GameHeap_Free).
void Texture_DestroyArray(u8 *array);
