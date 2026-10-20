// The texture manager: its release, a texture's release, its load flags setter, a texture's load and its lookup by name:
// reimplementation of 0x574b10, 0x575ad0, 0x575b80, 0x5755f0 and 0x575870.
//
// The texture manager is the object at [0x87a268]. Its +0x10008 holds the flags for the next
// texture it loads, which the load (0x5755f0) reads and clears; 2 asks for mip levels (how many
// by the device's +0x24, 0x575625). A texture set's load (0x575e2e) sets 2, 4 and 8 for its
// flags 0x1000, 0x2000 and 0x4000 one after the other, so only the last of them counts (sic).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x574b10 (cdecl; name unconfirmed): 0 without a texture manager ([0x87a268]). Else for each of its slots (0x10 bytes from +8;
// the count at +0x10000, signed and read again each time) the texture array at +0, when set, is released (Texture_Release on its
// first texture, gfx/texture), then destroyed (Texture_DestroyArray: released again, sic) and cleared, and +4 cleared; the count is
// cleared, the manager freed (GameHeap_Free) and [0x87a268] cleared. Returns 1.
u32 __cdecl TexManager_Destroy();

// 0x575ad0 (thiscall; textures; pops 4; name unconfirmed): the slots (0x10 bytes from +0: +8 a texture array, +0xc its users) are
// searched from the last (count at +0x10000, signed) for this array: none, 0. Found, its users are decremented: still some, 0. Else
// Texture_Release on its first texture, the array (when set) destroyed (Texture_DestroyArray), the count decremented and the last
// slot copied over this one (a dword at a time). Returns 1.
u32 __fastcall TexManager_ReleaseTexture(u8 *manager, void *, u8 *textures);

// 0x575b80 (thiscall; flags): +0x10008 = flags, replacing what was there.
void __fastcall TexManager_SetLoadFlags(u8 *manager, void *, u32 flags);

// 0x5755f0 (thiscall; path; pops 4; name unconfirmed): the mip levels after the first ([0x87a264]) 0, the load flags (+0x10008)
// taken and cleared; with flag 2 the device kind ([0x879ab0]+0x24) 0 clears it, 1 and 2 make 2 levels, 3 one. An array of levels + 1
// textures (count-prefixed, GameHeap_Alloc; not got: 0) built (CRT_VectorVbaseCtorIterator, Texture_Ctor), the file read (gfx/texfile's
// TexFile_Load with the flags; none: the array destroyed, 0). The format from what the read found (0x87a24c): 0x1000000, 0x11000000
// for a WCT image (4); then 0x30400 for a mask (8), else 0x8030000 for flag 4, else 0x200 for 2, else 0x100 for 1; 0x300 for flag 8,
// 0x40000 for 0x20, and 0x40 / 0x80 / 0xc0 for 0x100 / 0x200 / 0x300 of it. Each texture (levels re-read) gets the format (0x800000
// for all but the last), its pixels at +0x40 (the WCT image's virtual base [0x87a254], else the file's pixels a level apart: pixel
// size (0x7016f8[[0x8bd554]]) * width * height), Texture_SetSize with the file's width and height (the same for each level, sic) and
// the u16 width at +0x34 (flag 1's raw read sets neither: whatever the stack held, sic). Returns the array.
u8 *__fastcall TexManager_Load(u8 *manager, void *, const char *name);

// 0x575870 (thiscall; folder, name; pops 8; name unconfirmed): the slots searched from the last (count at +0x10000, signed) for the
// folder (+0: the same pointer, or both set and equal strings) and the name (+4, equal strings): found, its users (+0xc) raised and
// its textures (+8) returned. Else, with 0x1000 slots or more, 0; otherwise the path [0x786de4] + folder (when set; then a
// backslash unless it ends in one or is empty) + name (0x104 bytes, unchecked) loaded (TexManager_Load): got, a new slot takes the
// folder and name pointers, the textures and 1 user, and the textures are returned; not, the fallback slot (+0x10004; -1 none: 0)
// gets a user more and its textures are returned.
u8 *__fastcall TexManager_Acquire(u8 *manager, void *, const char *folder, const char *name);
}
