// The texture manager's load flags setter: reimplementation of 0x575b80.
//
// The texture manager is the object at [0x87a268]. Its +0x10008 holds the flags for the next
// texture it loads, which the load (0x575600) reads and clears; 2 asks for mip levels (how many
// by the device's +0x24, 0x575625). A texture set's load (0x575e2e) sets 2, 4 and 8 for its
// flags 0x1000, 0x2000 and 0x4000 one after the other, so only the last of them counts (sic).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x575b80 (thiscall; flags): +0x10008 = flags, replacing what was there.
void __fastcall TexManager_SetLoadFlags(u8 *manager, void *, u32 flags);
}
