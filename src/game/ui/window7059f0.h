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

// 0x663085 (thiscall; name unconfirmed): the items made free, each (+0x164, signed) + 4 dwords: the dword count +0x158 / that (signed;
// 0 faults, sic) items chained by their next sibling (+0), with no first child (-1) and the parent -2 (free); the last one's next -1.
void __fastcall Window7059f0_ResetItems(u8 *window);

// 0x663cb8 (thiscall (end, last), pops 8; name unconfirmed): with the array +0x144 set, from the first shown item (Window7059f0_NthItem
// 0) along the next siblings to the first that is -1 or less or free (parent -2): it is stored through end and the one before it (-1 for
// none) through last; neither is stored without the array.
void __fastcall Window7059f0_RootEnd(u8 *window, void *, s32 *end, s32 *last);

// 0x664495 (cdecl (bar, count, rows, first); name unconfirmed): with a scroll bar (kind 11), more items than rows (signed) shows it
// (Window_SetShown), sets its range to count - rows, 0 (Window705b20_SetRange) and gives first to its slot 7 (thiscall); else it is
// hidden and its range 0, 0.
void __cdecl Window7059f0_FitBar(u8 *bar, s32 count, s32 rows, u32 first);

// 0x664605 (thiscall (unused, item, previous, siblings), pops 16; name unconfirmed): the item freed: first its first child (previous -1)
// and, with siblings, its next sibling (previous the item), each with its siblings; each column's cell (the item's dword +4 + column)
// whose bottom (Window7059f0_ColumnBottom) is 0 deleted when set (CountedBuf_Deleter 1). Then, unless the item is the last one before
// the free ones (Window7059f0_RootEnd), it is unlinked (previous's next sibling its own with previous 0 or more, else its parent's first
// child -1, its other children dropped, sic) and put after that last one (it -1: the dword before the array, sic), the first free one
// its next. Its child -1, its parent -2 (free); the count +0x150 one less, the first shown +0x160 -1 at 0.
void __fastcall Window7059f0_FreeItem(u8 *window, void *, u32 unused, s32 item, s32 previous, u32 siblings);

// 0x66525c (thiscall (row), pops 4; name unconfirmed): the row selected: below 0 none (-1); one below the rows shown (+0x172, signed)
// and the count +0x150 the item +0x154 + row; another none with flag 0x100 of +0x48, else nothing. When that is not the selection +0x14c,
// the highlight object +0x174 (when set) is hidden (its slot 5, 0) going to none, shown (1) going from none while the window is shown
// (Window_IsShown); the selection set, its parent (+0x60) posted 0x401 (the id +0x8c, the selection) and, with the dword 0xfaa5d4 set,
// the window posted 3 (0, 0) (Window_PostMessage). Then with a highlight and a selection, the highlight's slot 3 gets 0x10 and an event
// whose +0x14 points at the selection's row rectangle (GuiRect_CtorWith): left +0x13c, right +0x140, top +0x13e + the row height +0x170
// * (selection - +0x154), the bottom one row further (32-bit, wrapping).
void __fastcall Window7059f0_Select(u8 *window, void *, s32 row);

// 0x664db7 (thiscall (text, column), pops 8; name unconfirmed): the position (from 0) of the first item from the first shown one whose
// cell in the column (1 to the columns +0x164, clamped; from 1) is a wide string equal to text (WString_CompareText), stepping as
// Window7059f0_FindItemData; -1 when none, without columns +0x16c or when that column's bottom (Window7059f0_ColumnBottom) is not 0.
s32 __fastcall Window7059f0_FindText(u8 *window, void *, const u16 *text, s32 column);

// 0x663324 (thiscall; name unconfirmed): with the row window array +0x148 set, the rows (+0x172, signed) refreshed from the item +0x154
// on (Window7059f0_NthItem), its level the number of its parents: each row window shown (Window_SetShown) when the list is
// (Window_IsShown) and the row is below the count +0x150 (row + +0x154, signed); for such a row each of the columns (+0x164, signed),
// walking the row window, its first child (Fn_65d547) and that one's siblings (Fn_65d56b), is sent message 0x403 (the item's dword +8's
// address, its data +1) and given the item's dword +4 + column: through its slot 7 (thiscall) when the column's bottom
// (Window7059f0_ColumnBottom) is 1, else as that dword's object's +4 (Obj_Get4Or6fd6c4) through its slot 9, the first column told the level
// * 0x14 and 0 first (Window_NotifyC000). Then the next item (Window7059f0_NextItem), its level change added.
void __fastcall Window7059f0_RefreshRows(u8 *window);

// 0x6647cb (thiscall (n), pops 4; name unconfirmed): 0 for n at or past the count +0x150, without items +0x144 or below 0. Else the n-th
// item (Window7059f0_NthItem) freed with its children (Window7059f0_FreeItem, n, previous, no siblings): previous -1 for the first one
// (whose next sibling becomes the first shown +0x160) and for its parent's first child, else the (n - 1)-th item of the walk (not always a
// sibling, sic). The selection +0x14c when it was n: one less past the new count, -1 without items (Window7059f0_Select). With the dword
// 0xfaa5d4 set and n among the rows shown (+0x154 to +0x154 + +0x172), the window posted 3 (0, 0); the bar (child 1, Window_FindChild)
// fitted (Window7059f0_FitBar), the parent +0x60 sent 0x405 (+0x154, the last row shown, below the count) and the rows refreshed
// (Window7059f0_RefreshRows). 1 in AX (the rest of EAX the last call's).
u32 __fastcall Window7059f0_RemoveItem(u8 *window, void *, s32 n);

// 0x6649d5 (thiscall; name unconfirmed): nothing without items (the count +0x150 0). Else each item's cells (the n-th item's for n below
// the count) in the columns whose bottom (Window7059f0_ColumnBottom) is 0 deleted when set (CountedBuf_Deleter 1), the links reset
// (Window7059f0_ResetItems), the window posted 3 (0, 0) with the dword 0xfaa5d4 set, the count, the selection +0x14c (-1), the first shown
// +0x160 (-1) and the first row +0x154 cleared, the bar (child 1) fitted, the highlight +0x174 (when set) hidden (its slot 5, 0) and the
// rows refreshed.
void __fastcall Window7059f0_Clear(u8 *window);

// 0x665739 (thiscall (position), pops 4; name unconfirmed): nothing without items +0x144. Below 0 the selection cleared (Select -1); else
// the position (at most the count - 1) scrolled into the rows (+0x154 the position when outside them, then at most count - rows) and
// selected (Select, the row on screen). Then the rows refreshed, the bar (child 1, Window_FindChild; none a fault, sic) given +0x154
// through its slot 7 and, when +0x154 changed, the parent +0x60 sent 0x405 (+0x154, the last row shown, below the count).
void __fastcall Window7059f0_ScrollTo(u8 *window, void *, s32 position);
}
