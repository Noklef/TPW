// The GUI library's window subclass with the vtable 0x705c58 (kind 6 of the factory 0x65dd7b: nothing past Window_Ctor's fields; what
// it is is unconfirmed). Reimplementation of 0x66d9e0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66d9e0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705c58. Returns the window.
u8 *__fastcall Window705c58_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
