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

// 0x66274e (thiscall; name unconfirmed): the destructor: the vtable 0x7059f0, Window7059f0_Clear; the column array +0x16c (when set)
// deleted by its slot 4 (3: the GUI rectangles' vector deleting destructor); with the row array +0x148 set, each row window there (up to
// the rows shown +0x172, signed) deleted by its slot 2 (1) and the array freed (Mem_Free); the items +0x144 freed when set; the highlight
// +0x174 let go (WindowImpl_Release), and when that answers -1 (as a word) its slot 0 (1) called; the rectangle +0x138 destroyed
// (GuiRect_Dtor), then Window_Dtor, whose EAX is left.
u32 __fastcall Window7059f0_Dtor(u8 *window);

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

// 0x663718 (thiscall (row: a window), pops 4; name unconfirmed): for a row given, the row height +0x170 = its inner height (Window_GetSize
// (inner 1)), the rows shown +0x172 = (the words +0x142 - +0x13e) / it (signed; a height of 0 faults), the row window array +0x148 made
// when there is none (Mem_Alloc of rows * 4); with the highlight +0x174 set, it is told by slot 3 (0x10, an event on the stack whose +0x14
// points to a GUI rectangle +0x13c, +0x13e, +0x140, +0x13e + height (GuiRect_CtorWith) and whose +0x24 is 0x10; the rest unset). The rows
// shown (AX) either way.
u16 __fastcall Window7059f0_SetRowSize(u8 *window, void *, u8 *row);

// 0x663864 (thiscall (index, row: a window), pops 8; name unconfirmed): with a row given, the array +0x148 and the columns +0x16c there
// and index below the rows shown (+0x172, signed: a negative index writes before the array, sic), the row stored at index and laid out:
// for each column (+0x164, signed) the window (the row, then its first child (Fn_65d547), then that one's siblings (Fn_65d56b)) set
// (Window_SetRectTo) to the column's left / right words (+4 / +8) and from +0x13e + index * +0x170 (a word) down by +0x170, and given the
// depth (Window_SetDepth) of the list's word +0xe0 + 3. A GUI rectangle is made and dropped unused. EAX not kept.
void __fastcall Window7059f0_SetRow(u8 *window, void *, s32 index, u8 *row);

// 0x662eee (thiscall (highlight: an object), pops 4; name unconfirmed): nothing when it is the highlight +0x174 already; else the old one
// let go (WindowImpl_Release), and when that answers -1 (as a word) its slot 0 (1) called. +0x174 = highlight; when set: Fn_6617f0 on it,
// then its slot 3 (0x17, an event on the stack: the words +4 = 0, +6 = the list's word +0xe0 + 2, +8 = its +0xe2, +0x14 a GUI rectangle
// +0x13c, +0x13e, +0x140, +0x13e + the row height +0x170 (GuiRect_CtorWith), +0x24 = 0x17; the rest unset), then, with the list shown
// (Window_IsShown) and its +0x14c not -1, its slot 5 (1). EAX not kept.
void __fastcall Window7059f0_SetHighlight(u8 *window, void *, u8 *highlight);

// 0x664fde (thiscall (shown: a word), pops 4; name unconfirmed): with the row array +0x148, each row window there (up to the rows shown
// +0x172, signed; 0 skipped) shown (Window_SetShown) when shown is not 0 and its item (the row + the first shown +0x154, wrapping) is below
// the count +0x150 (signed), else hidden; the highlight +0x174 (when set) told by its slot 5 (1 when +0x14c is not -1 and shown not 0,
// else 0). EAX not kept.
void __fastcall Window7059f0_ShowRows(u8 *window, void *, u32 shown);

// 0x6650d4 (thiscall (message, a, b), pops 12; name unconfirmed): with the row array +0x148, the message sent (Window_SendMessage) to
// every window of each row there (up to the rows shown +0x172, signed; 0 skipped): the row, its first child (Fn_65d547), that one's
// siblings (Fn_65d56b), one per column (+0x164, signed). For 0x1e, a is row << 16 | column and b the data dword (+1) of the row's item:
// from the first shown one (Window7059f0_NthItem(0)) on, one per row given while there is one (not -1; Window7059f0_NextItem), b kept
// from the row before when the items have run out. EAX not kept.
void __fastcall Window7059f0_SendToRows(u8 *window, void *, u32 message, u32 a, u32 b);

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

// 0x663c11 (thiscall (n, parent), pops 8; name unconfirmed): the item n next siblings (+0) along from the first child (+2) of the
// parent-th item (Window7059f0_NthItem; -1 when that gives -1 or less), or, for a parent of -1 or less, from the first shown (+0x160);
// stepping only for n from 0 to below the count +0x150 (signed) with the array +0x144 set, and while the item is above -1.
s32 __fastcall Window7059f0_NthChild(u8 *window, void *, s32 n, s32 parent);

// 0x6628f4 (thiscall (unused), pops 4; name unconfirmed): with the columns +0x16c set and 2 or more items (+0x150, signed), the top
// list of siblings from the first shown (+0x160) sorted by the column +0x168 (a word: the cells are the dwords +4 + column from each
// item): as numbers (signed) when that column's bottom word (+0xa) is 1, else as the strings they point to (WString_Compare); ascending
// for the word +0x16a 0, descending for 1, else nothing moves. First, over at most count items to -1 or one whose parent (+3) is -2,
// the first coming-first one becomes the first shown (unlinked after its previous, linked before the old first); then, with more than
// 2 items, an insertion pass: from the third (Window7059f0_NthChild) while below the count, its parent not -2 and it 0 or more, each
// moves before the first item from the second on that it comes before, the search stopping at it or at an item whose parent is -2
// or less.
void __fastcall Window7059f0_Sort(u8 *window, void *, s32 unused);

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

// 0x66316b (thiscall (n), pops 4; name unconfirmed): 0 for n 0 or no columns (+0x164, signed). Else the item array +0x144 grown by n items
// of columns + 4 dwords: a block of (the dword count +0x158 + that) * 4 bytes (Mem_Alloc; none: 0) cleared, the new items from the old
// count on chained as free ones (each the next one's previous, child -1, parent -2; the last's next -1); with an old array, its dwords
// copied over, the dword of the last item before the first free one (Window7059f0_RootEnd, taken before the allocation; -1: the dword
// before the block, sic) set to the old count and the old array freed (Mem_Free). The block becomes +0x144, the count raised; returns it.
s32 *__fastcall Window7059f0_Grow(u8 *window, void *, s32 n);

// 0x663dfa (thiscall (texts, cells), pops 8; name unconfirmed): for each column (+0x164, signed), the next cell: a column whose bottom
// (Window7059f0_ColumnBottom) is not 0 gets the text's dword as it is; else a set text becomes a new wide string (Mem_Alloc(0x10),
// StdWString_CtorChars; none: 0) whose allocator byte is that of a local never set (sic), and an unset one leaves the cell as it was.
void __fastcall Window7059f0_CopyCells(u8 *window, void *, const u32 *texts, u32 *cells);

// 0x663edc (thiscall (texts, cells), pops 8; name unconfirmed): whether a new item's texts go before an item's cells in the sort column
// +0x168 (signed): with +0x16a 0 (ascending), for a column whose bottom (Window7059f0_ColumnBottom) is 1 the dwords compared signed
// (texts' below), else 1 for an empty cell or the text below the cell's string (wcscmp, the cell through Obj_Get4Or6fd6c4); with +0x16a
// set the same the other way round (the dword above, the cell's string below the text; an empty cell 1 too).
u32 __fastcall Window7059f0_SortsBefore(u8 *window, void *, const u32 *texts, const u32 *cells);

// 0x66403b (thiscall (texts, data, parent, refresh), pops 16; name unconfirmed): -1 without texts. Else an item added, the array grown
// first when the count +0x150 times the columns + 4 reaches the dword count +0x158 (Window7059f0_Grow 20, or 10 with an array). Under the
// parent (a position below the count, else -1: the roots) whose first child, or the first shown +0x160, is looked at: none: the first free
// item (Window7059f0_RootEnd) becomes the parent's only child (unlinked from the free chain), or without a parent item 0 takes the cells
// and becomes the first shown, unlinked (sic). Else the first free item gets the cells (Window7059f0_CopyCells) and, with flag 0x10 of
// +0x48 (sorted), is moved before the first of the siblings from there (at most count - 1 steps) it sorts before
// (Window7059f0_SortsBefore) or to their end; unsorted it stays where it is, after the roots even under a parent (sic). Its parent -1
// when below 0, its data set, the count raised (the selection 0 at 1, Window7059f0_Select); with refresh the rows refreshed and, with the
// dword 0xfaa5d4 set, the window posted 3 (0, 0); the bar fitted and the parent +0x60 sent 0x405. Returns 0.
s32 __fastcall Window7059f0_AddItem(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);

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

// 0x664ea3 (thiscall; name unconfirmed): with the bar (child 1, Window_FindChild), its slot 8's answer as the first row shown: when it
// differs from +0x154, +0x154 = it, the rows refreshed (Window7059f0_RefreshRows), the parent +0x60 sent 0x405 (+0x154, the last row
// shown, below the count +0x150) and, with [0xfaa5d4] set, the list posted 3 (0, 0). EAX not kept.
void __fastcall Window7059f0_ScrollFromBar(u8 *window);
}
