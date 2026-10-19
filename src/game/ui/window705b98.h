// The GUI library's window subclass with the vtable 0x705b98 (kind 13 of the factory 0x65dd7b: a dword +0x138; what it is is
// unconfirmed). Reimplementation of 0x66c7e0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66c7e0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705b98 and +0x138 0. Returns the window.
u8 *__fastcall Window705b98_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
