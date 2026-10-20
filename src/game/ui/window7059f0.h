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

// The items: a tree in the dword array +0x144, each item four dwords from its index: +0 the next sibling, +2 the first child, +3 the
// parent (-1 or less: none); +0x150 the item count, +0x160 the first shown.

// 0x6636f6 (thiscall (column: a word), pops 4; name unconfirmed): the bottom word (+0xa, sign-extended) of the column's rectangle in the
// array +0x16c (0xc bytes each).
s32 __fastcall Window7059f0_ColumnBottom(u8 *window, void *, u32 column);

// 0x663a93 (thiscall (item, level), pops 8; name unconfirmed): the item after the given one in the tree's order: its first child (level
// 1), else its next sibling (0), else its parent's next sibling, going up while a parent has none but a parent of its own (level -1 for
// each step up; the last one's next sibling, -1 or less, when none has one); -2 (level 0) with no sibling and no parent. The level
// change is stored through level when it is not 0.
s32 __fastcall Window7059f0_NextItem(u8 *window, void *, s32 item, s32 *level);

// 0x663ba2 (thiscall (n), pops 4; name unconfirmed): the n-th item after the first shown one (+0x160), stepping Window7059f0_NextItem
// while n is 0 to below the count +0x150 (signed), the array +0x144 is set and no step gave -1; the first shown one otherwise.
s32 __fastcall Window7059f0_NthItem(u8 *window, void *, s32 n);

// 0x664b68 (thiscall (n, data), pops 8; name unconfirmed): the n-th item's (Window7059f0_NthItem) data dword (+1) set; 1 in AX, 0 when
// that gave -1.
u32 __fastcall Window7059f0_SetNthItemData(u8 *window, void *, s32 n, u32 data);

// 0x664ba8 (thiscall (height: a word), pops 4; name unconfirmed): the row height word +0x170 = height and the rows shown +0x172 = the
// inner rectangle's height (+0x142 - +0x13e, signed) / it (signed; 0 faults, sic).
void __fastcall Window7059f0_SetRowHeight(u8 *window, void *, u32 height);

// 0x664d2a (thiscall (data), pops 4; name unconfirmed): the position (from 0) of the first item from the first shown one whose data
// dword (+1) is data, stepping (Window7059f0_NextItem) until -1 or the position reaches the count +0x150 - 1 (signed); -1 when none.
s32 __fastcall Window7059f0_FindItemData(u8 *window, void *, u32 data);
}
