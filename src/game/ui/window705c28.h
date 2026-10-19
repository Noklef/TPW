// The GUI library's window subclass with the vtable 0x705c28 (kind 9 of the factory 0x65dd7b: a GUI rectangle +0x148, dwords +0x138-+0x140;
// what it is is unconfirmed). Reimplementation of 0x66d750. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66d750 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then a GUI rectangle
// constructed at +0x148 (GuiRect_Ctor), the vtable 0x705c28, +0x140 / +0x13c / +0x138 0, the saved rectangle +0x2c set from +8 and +0x148
// from that (GuiRect_Assign). Returns the window.
u8 *__fastcall Window705c28_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
