// The advisor's record arrays' element constructors (names invented): the unported constructor at 0x5187xx (the same layout as
// Advisor_Ctor's, world/advisor) builds 8 records of 0x18 bytes at +0x14 and 0x15f records of 0x10 bytes at +0xe0 through the array
// constructor (0x401000) with these. Reimplementation of 0x518e60 and 0x518e80.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x518e60 (thiscall): the byte +0x14 cleared. Returns the record.
u8 *__fastcall AdvisorSlot_Ctor(u8 *slot);
// 0x518e80 (thiscall): +0 = 0, +4 = -1, the byte +8 = 0, +0xc = 0. Returns the record.
u8 *__fastcall AdvisorRecord_Ctor(u8 *record);

}  // extern "C"
