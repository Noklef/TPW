// The GUI library's window subclass with the vtable 0x705b20 (built by 0x66acf9 over Window_Ctor: a step +0x144 of 1, a word of flags
// +0x14c 1 or 2; 0x66b557 makes it child windows of the factory's kind 2, laid out by flag 2: a scroll bar, unconfirmed), its range and
// enable. Reimplementation of 0x66b47d and 0x66ae83. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66b47d (thiscall (from, to), pops 8; name unconfirmed): nothing when +0x138 is `to` and +0x13c `from` already. Otherwise the step
// +0x144 made to point from `from` to `to`: negated when it is above 0 and to < from, or below 0 and to >= from (signed); then +0x13c =
// from and +0x138 = to.
void __fastcall Window705b20_SetRange(u8 *window, void *, s32 from, s32 to);

// 0x66ae83 (thiscall (enabled: a word), pops 4; slot 5 of the vtable 0x705b20; name unconfirmed): 0 (AX) when flag 2 of +0x48 (disabled)
// already says so. Otherwise Window_SetEnabled, each child (from +0x5c, the next by Fn_65d56b) gets slot 5 (enabled), the window is
// posted 1 (0, 0) (Window_PostMessage) when the dword [0xfaa5d4] is set, and 1 (AX).
u16 __fastcall Window705b20_SetEnabled(u8 *window, void *, u32 enabled);
}
