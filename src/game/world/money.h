// The money record's changes (the object at 0x7cdb30 is one): reimplementation of 0x4d0190 and 0x4d01f0. Named by what they do until
// the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d0190 (thiscall; amount, pops 4): the balance at +0xc and the total at +0x124 gain amount, and so does the dword at +0x1fc90 of the
// thing the game object's (the dword at 0x7cf83c) handle at +0x1da720 names (the debug print of an amount over 1000000 is not
// reproduced).
void __fastcall Money_Credit(u8 *money, void *, s32 amount);

// 0x4d01f0 (thiscall; amount, pops 4): nothing unless the dword at +0x114 is set; then the balance at +0xc loses amount and is copied to +0x11c; going below 0
// while the dword there was not below 0 records the time (the dword at +0x1da70c of the object [0x80239c]) at +0x120; the dword at +0x1f5a0 of that thing gains amount and the total at +0x124 loses it.
void __fastcall Money_Debit(u8 *money, void *, s32 amount);

}  // extern "C"
