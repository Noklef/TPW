// The game's error report (the object 0x405750 shows: +0 set, +4 the error's index 0-0x2a into the
// string table's messages, +8, +0xc a text of 0x50): reimplementation of 0x405b90 and 0x405710.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x405b90 (thiscall, the report unused; code, pops 4; name unconfirmed): the error index of a library code:
// 0xb0754000-0xb0754018 give 0xe-0x26 in turn, 0xb0754100 / 0xb0754101 / 0xb0754102 give 0x27 / 0x28 / 0x29,
// any other 0x2a.
u32 __fastcall GameError_IndexOfCode(void *report, void *, u32 code);

// 0x405710 (thiscall; index, arg, text, pops 12; name unconfirmed): the report set: +4 the index, +8 arg, the text (when given) into +0xc
// (strncpy of 0x50: no NUL when it is that long), then +0 = 1.
void __fastcall GameError_Set(u8 *report, void *, u32 index, u32 arg, const char *text);
}
