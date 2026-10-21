// The GUI library's window subclass with the vtable 0x705b98 (kind 13 of the factory 0x65dd7b: a dword +0x138; what it is is
// unconfirmed: bits +0x138 that show their children, ids 0x11100000 | the bit). Reimplementation of 0x66c7e0 and 0x66c85a. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66c7e0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705b98 and +0x138 0. Returns the window.
u8 *__fastcall Window705b98_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x66c85a (thiscall (bits), pops 4; name unconfirmed): for each bit 0-30 that differs between +0x138 and bits, the child of id
// 0x11100000 | the bit's number (Window_FindChild), when there is one, told 1 (newly set) or 0 (newly clear) (Window_Notify1); then
// +0x138 = bits and Window_Notify80(bits). 1 (AX).
u16 __fastcall Window705b98_SetBits(u8 *window, void *, u32 bits);
}
