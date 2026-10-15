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
}
