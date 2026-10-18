// Litter on the park's cells. Reimplementation of 0x4d9620 and 0x4d9600. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d9620 (thiscall on the litter {+0 cell id word, +0x28 sprite script, +0x2c kind, +0x30 word, +0x38 particle system handle}; the debug prints are left out): kind 7 spreads
// record 2 back out of the cell (Map_SpreadRemove on [0x8023a0]); kind 8 record 5 and then ends the particle system when it has a handle; then the sprite script is killed and
// the script, the word at +0x30 and the kind cleared.
void __fastcall Litter_Remove(u8 *litter);

// 0x4d9600 (thiscall; word, pops 4; on a litter record of 0x44 bytes, from [0x8023a0], as the handyman's update 0x4d7060 calls it with
// its thing's handle): the word at +0x30 = word and the dword +0x34 = the dword at +0x1da70c of the object at [0x80239c] (the world's
// clock); returns that dword.
u32 __fastcall Obj_StampAt30(u8 *object, void *, u32 word);
}  // extern "C"
