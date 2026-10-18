// The frame library's base object (vtable 0x705a80, built by 0x669fa8: +4 a word 1, +6 0, +0xc a word, +0x10 a time, +0x14 a dword; the help
// background derives from it): reimplementation of 0x66a084 and 0x66a016. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66a084 (thiscall (value), pops 4; slot 9): +0x14 = value; when the words +4 and +0xc are both set, +0x10 = the GUI's time now
// (GuiClock_Now).
void __fastcall Frame_SetInterval(u8 *frame, void *, u32 value);

// 0x66a016 (thiscall; name unconfirmed): the word +4 = 0, then with the word +6 set the object's slot 1, whose result is returned (else the frame).
u32 __fastcall Frame_Stop(u8 *frame);
}
