// The GUI library's window subclass with the vtable 0x705bf8 (kind 10 of the factory 0x65dd7b: dwords +0x138-+0x148 and words +0x14c /
// +0x14e, the handler 0x66d6f5; what it is is unconfirmed). Reimplementation of 0x66d0c0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66d0c0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705bf8, +0x148 / +0x138 / +0x13c / +0x140 / +0x144 0, the words +0x14c / +0x14e 0, the saved rectangle +0x2c set from +8
// (GuiRect_Assign) and the handler 0x66d6f5 (Window_SetHandler). Returns the window.
u8 *__fastcall Window705bf8_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
