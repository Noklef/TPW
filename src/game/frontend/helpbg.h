// The help background (f_helpbg; vtable 0x6ff598, base 0x705a80): a frame-library window behind the help text. Reimplementation of 0x492320, 0x4922d0
// 0x491fe0 and 0x492160.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492320 (thiscall; slot 7 of the help background's vtable 0x6ff598; name unconfirmed): when the word at +4 is set, the object's virtual method 0 (a tail call).
void __fastcall HelpBg_Refresh(u8 *self);

// 0x4922d0 (thiscall; slot 15 of the help background's vtable 0x6ff598): +0x18 = 0 and the word +0xc = 0; returns 0.
u32 __fastcall Obj_Clear18_0c(u8 *object);

// 0x491fe0 (cdecl (window, message, a, b); the handler of the help background's window, installed at 0x4922b0; name unconfirmed): on message 0x14
// the window's object (Window_DwordA0 0, the help background) gets its slot 15 (Obj_Clear18_0c); then Window_CallDefault with the message,
// its result returned.
s32 __cdecl HelpBg_WindowHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x492160 (thiscall (value), pops 4; slot 9 of 0x6ff598; name unconfirmed): only while the word +0xc is set: the base's
// slot 9 (0x66a084, ui/frame), then the object's slot 0.
void __fastcall HelpBg_SetInterval(u8 *self, void *, u32 value);
}  // extern "C"
