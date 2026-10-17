// The texture manager's load flags setter, a texture's release and the manager's release: reimplementation of 0x574b10, 0x575ad0
// and 0x575b80.
//
// The texture manager is the object at [0x87a268]. Its +0x10008 holds the flags for the next
// texture it loads, which the load (0x575600) reads and clears; 2 asks for mip levels (how many
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
}
