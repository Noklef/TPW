// The GUI library's window subclass with the vtable 0x705b20 (built by 0x66acf9 over Window_Ctor: a step +0x144 of 1, a word of flags
// +0x14c 1 or 2; 0x66b557 makes it child windows of the factory's kind 2, laid out by flag 2: a scroll bar, unconfirmed), its range.
// Reimplementation of 0x66b47d. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66b47d (thiscall (from, to), pops 8; name unconfirmed): nothing when +0x138 is `to` and +0x13c `from` already. Otherwise the step
// +0x144 made to point from `from` to `to`: negated when it is above 0 and to < from, or below 0 and to >= from (signed); then +0x13c =
// from and +0x138 = to.
void __fastcall Window705b20_SetRange(u8 *window, void *, s32 from, s32 to);
}
