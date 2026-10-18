// The game's error report (the object 0x405750 shows: +0 set, +4 the error's index 0-0x2a into the
// string table's messages, +8): reimplementation of 0x405b90.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x405b90 (thiscall, the report unused; code, pops 4; name unconfirmed): the error index of a library code:
// 0xb0754000-0xb0754018 give 0xe-0x26 in turn, 0xb0754100 / 0xb0754101 / 0xb0754102 give 0x27 / 0x28 / 0x29,
// any other 0x2a.
u32 __fastcall GameError_IndexOfCode(void *report, void *, u32 code);
}
