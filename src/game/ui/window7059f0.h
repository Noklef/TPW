// The GUI library's window subclass with the vtable 0x7059f0 (kind 7 of the factory 0x65dd7b: a GUI rectangle +0x138 the saved one, a
// scroll bar (the factory's kind 3) made with flag 0x1000, the handler 0x665c35; what it is is unconfirmed). Reimplementation of 0x662530
// and 0x662562. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x662530 (thiscall; name unconfirmed): the saved rectangle +0x2c set from +8 and the rectangle +0x138 from that (GuiRect_Assign).
void __fastcall Window7059f0_Layout(u8 *window);

// 0x662562 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, the rectangle +0x138
// (GuiRect_Ctor), the vtable 0x7059f0, +0x14c and +0x160 -1, the dwords +0x144, +0x148, +0x150-+0x158, +0x16c and +0x174 and the words
// +0x164-+0x16a, +0x170 and +0x172 0; Window7059f0_Layout; with flags & 0x1000 a child of the factory's kind 3 (flags 0x10, and 0x40 with
// flags & 0x40; id 1) from +0x34 + 4 / +0xe to +0x10 / +0x12 that, when made, gets slot 5 (0), Window_SetShown(1) and
// Window705b20_SetRange(0, 0); then the handlers +0x114 / +0x118 0x665c35. Returns the window.
u8 *__fastcall Window7059f0_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);
}
