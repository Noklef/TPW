// The highlight of a part by selection kind: reimplementation of 0x598ca0. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x598ca0 (cdecl; kind): on the model slot at [0xf79674] the part flag of the parts named by the kind: -1 clears the flags of the keys at 0xf797a4 (while bit 0x10000 of [0xf79660]
// is clear, each when set) and sets those of the keys at 0xf797b4 (four each); 0 clears 0x13 and 0x14; 1, 2, 4, 5, 6, 7, 9, 10, 11, 13 set 0x13 and 0x14; 14 sets 0x15; other kinds
// do nothing here. Finally, for a kind other than 0 and while that bit is clear, the flag of the part with the kind as its key is cleared (ModelSlot_ClearPartFlag).
void __cdecl ModelSlot_ApplySelection(s32 kind);

}  // extern "C"
