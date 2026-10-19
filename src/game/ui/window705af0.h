// The GUI library's window subclass with the vtable 0x705af0 (kind 12 of the factory 0x65dd7b: dwords +0x138-+0x148, +0x140 the address
// 0x9708cd, the handler 0x66aa61; what it is is unconfirmed). Reimplementation of 0x66a4c0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66a4c0 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705af0, +0x138 0, +0x13c 1, +0x140 0x9708cd, +0x144 0, +0x148 1, the byte +0x154 1, the saved rectangle +0x2c set from +8
// (GuiRect_Assign) and the handler 0x66aa61 (Window_SetHandler). Returns the window.
u8 *__fastcall Window705af0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
