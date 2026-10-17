// A table of model slots (0xb0 bytes each at 0x790398; +4 a holder that is also its
// animator), used by the 0x54e000 and 0x598000 front-end code: reimplementations of
// 0x429df0, 0x429ee0, 0x429f60 and 0x429fe0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x429df0: Animator_Update(holder, 1, 0) when the slot has a holder.
void __cdecl ModelSlot_Animate(u32 slot);

// 0x429ee0 / 0x429f60: clear / set flag 0x10 of the face of the slot's first part with key
// (+4) and flag 0x400 (Model_FindPartByKey4), when there is one.
void __cdecl ModelSlot_ClearPartFlag(u32 key, u32 slot);
void __cdecl ModelSlot_SetPartFlag(u32 key, u32 slot);

// 0x429fe0: the slot's holder.
u8 *__cdecl ModelSlot_Holder(u32 slot);

// 0x598ca0 (cdecl; kind): on the model slot at [0xf79674] the part flag of the parts named by the kind: -1 clears the flags of the keys at 0xf797a4 (while bit 0x10000 of [0xf79660]
// is clear, each when set) and sets those of the keys at 0xf797b4 (four each); 0 clears 0x13 and 0x14; 1, 2, 4, 5, 6, 7, 9, 10, 11, 13 set 0x13 and 0x14; 14 sets 0x15; other kinds
// do nothing here. Finally, for a kind other than 0 and while that bit is clear, the flag of the part with the kind as its key is cleared (ModelSlot_ClearPartFlag).
void __cdecl ModelSlot_ApplySelection(s32 kind);
}
