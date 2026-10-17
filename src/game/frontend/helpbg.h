// The help background (f_helpbg; vtable 0x6ff598, base 0x705a80): a frame-library window behind the help text.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492320 (thiscall; slot 7 of the help background's vtable 0x6ff598; name unconfirmed): when the word at +4 is set, the object's virtual method 0 (a tail call).
void __fastcall HelpBg_Refresh(u8 *self);
}  // extern "C"
