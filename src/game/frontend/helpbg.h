// The help background (f_helpbg; vtable 0x6ff598, base 0x705a80): a frame-library window behind the help text. Reimplementation of 0x492320 and 0x4922d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492320 (thiscall; slot 7 of the help background's vtable 0x6ff598; name unconfirmed): when the word at +4 is set, the object's virtual method 0 (a tail call).
void __fastcall HelpBg_Refresh(u8 *self);

// 0x4922d0 (thiscall; slot 15 of the help background's vtable 0x6ff598): +0x18 = 0 and the word +0xc = 0; returns 0.
u32 __fastcall Obj_Clear18_0c(u8 *object);
}  // extern "C"
