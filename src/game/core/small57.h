// Taking a piece of litter off its cell: reimplementation of 0x4d9620. Named by what it does until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d9620 (thiscall on the litter {+0 cell id word, +0x28 sprite script, +0x2c kind, +0x30 word, +0x38 particle system handle}; the debug prints are left out): kind 7 spreads
// record 2 back out of the cell (Map_SpreadRemove on [0x8023a0]); kind 8 record 5 and then ends the particle system when it has a handle; then the sprite script is killed and
// the script, the word at +0x30 and the kind cleared.
void __fastcall Litter_Remove(u8 *litter);

}  // extern "C"
