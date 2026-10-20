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

// 0x663554 (thiscall (count: a word), pops 4; name unconfirmed): only while there are no columns (the word +0x164 0): an array of count (signed)
// GUI rectangles (Mem_Alloc of count * 12 + 4, the count first; CRT_VectorCtorIterator with GuiRect_Ctor / GuiRect_Dtor) at +0x16c (0 when the
// allocation fails), column i set (Fn_659dac) to i * 32, 0, (i + 1) * 32, 1, and +0x164 = count.
void __fastcall Window7059f0_SetColumns(u8 *window, void *, u32 count);

// 0x663662 (thiscall (index, left, right: words), pops 12; name unconfirmed): for an index below the column count (signed) with the array there,
// the column's rectangle set (Fn_659dac) to left, 0, right, 1. EAX is left the rectangle, else the index (sign-extended) or the window.
u32 __fastcall Window7059f0_SetColumn(u8 *window, void *, u32 index, u32 left, u32 right);

// 0x6651ef (thiscall (left, top, right, bottom: words), pops 16; name unconfirmed): the rectangle +0x138's four words set (Fn_659dac), whose EAX
// is returned.
u32 __fastcall Window7059f0_SetInner(u8 *window, void *, u32 left, u32 top, u32 right, u32 bottom);

// 0x665bfb (thiscall; name unconfirmed): with the dword +0x148 set and the window it points at set, that window's depth word +0xe0
// (Fn_659730); else 0. Only AX is set.
u32 __fastcall Window7059f0_Item148Depth(const u8 *window);
}
