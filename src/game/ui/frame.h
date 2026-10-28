// The frame library's base object (vtable 0x705a80, built by 0x669fa8: +4 a word 1, +6 0, +0xc a word, +0x10 a time, +0x14 a dword; the help
// background derives from it): reimplementation of 0x66a084, 0x66a016, 0x66a0bb, 0x66a0f1, 0x66a148 and 0x66a180. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66a084 (thiscall (value), pops 4; slot 9): +0x14 = value; when the words +4 and +0xc are both set, +0x10 = the GUI's time now
// (GuiClock_Now).
void __fastcall Frame_SetInterval(u8 *frame, void *, u32 value);

// 0x66a016 (thiscall; name unconfirmed): the word +4 = 0, then with the word +6 set the object's slot 1, whose result is returned (else the frame).
u32 __fastcall Frame_Stop(u8 *frame);

// 0x66a0bb (thiscall; slot 10; name invented): +0x14 = 0, then when the words +4 and +0xc are both set the object's slot 1.
void __fastcall Frame_Expire(u8 *frame);

// 0x66a0f1 (thiscall; slot 11; name invented): the same code as Frame_Expire.
void __fastcall Frame_ExpireB(u8 *frame);

// 0x66a148 (thiscall (value), pops 4; slot 13; name invented): when +0x14 is value, +0x14 = 0 and the object's slot 1.
void __fastcall Frame_ExpireIf(u8 *frame, void *, u32 value);

// 0x66a180 (thiscall (flags), pops 4; slot 2; name invented): the destructor (Fn_669fee: the vtable 0x705a80 set), the block freed
// (GameHeap_Free) for flags bit 0; the frame returned.
u8 *__fastcall Frame_DeletingDtor(u8 *frame, void *, u32 flags);
}
