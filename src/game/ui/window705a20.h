// The GUI library's window subclass with the vtable 0x705a20 (kind 5 of the factory 0x65dd7b: with flag 0x40 a 0x200-byte buffer +0x138,
// a window timer +0x164 (0x2ee, id 0x75e88), the handler 0x66857d; what it is is unconfirmed, a text field going by the buffer and the
// timer). Reimplementation of 0x665fb0 and 0x665fcd. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x665fb0 (thiscall; name unconfirmed): the saved rectangle +0x2c set from +8 (GuiRect_Assign).
void __fastcall Window705a20_SaveRect(u8 *window);

// 0x665fcd (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705a20 and +0x13c 0; with flags & 0x40 +0x138 a new 0x200-byte block (Mem_Alloc) whose first word is 0 and +0x13c 0xff (none got:
// +0x138 0), without it +0x138 and +0x13c 0; Window705a20_SaveRect; +0x164 a window timer (WindowTimers_Add(window, 0x2ee, 0x75e88));
// the words +0x14a and +0x14c 0xffff, +0x148 and +0x14e-+0x158 0, the dwords +0x140, +0x144 and +0x15c 0, the byte +0x161 0; the
// handler 0x66857d (Window_SetHandler) and +0x118 too, Window_SetFocus(0); the byte +0x160 1 and +0x162 whether the byte 0xfaa56c is set.
// Returns the window.
u8 *__fastcall Window705a20_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
