// The nodes of the GUI library's list window (vtable 0x705950: see window705950.h). A node is 0x54 bytes: its text of 0x20 units at +0, the value +0x40, the next +0x44,
// the first child +0x48, the parent +0x4c, the previous +0x50. Reimplementation of 0x65ae3e, 0x65b20b, 0x65b486, 0x65b66a, 0x65b6d6, 0x65b79c, 0x65b7db. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65ae3e (thiscall (n, text, value), pops 12): -1 when the allocation fails or n is not below the count +0x14c (signed; the block then not freed, sic). Else a new node under the row n (at the top
// for n <= -1), its count taken: as the first of all when the list is empty; with flag 0x10 of +0x48 in rising order of CRT_wcscmp among the children, with 0x20 (and no 0x10) before the first it is not
// below, else at their end (a row with no children then faults, sic). The answer the node's place: 0 at the head, 1 for after the first, and so on (0 too in the empty list). A negative n other than -1 with
// no children to go in front of faults. With more rows than +0x156 the bar (child 1) shown, its range count - rows..0 (Window705b20_SetRange) and page rows (Fn_66b53e); with [0xfaa5d4] the window posted 3.
u16 __fastcall Window705950_InsertSorted(u8 *window, void *, s32 n, const u16 *text, u32 value);

// 0x65b20b (thiscall (n, position, text, value), pops 16): -1 when the allocation fails. position held in 0..count - 1 and n below the count. A new node under n (at the top for n <= -1) at position among
// the children: at the head (position 0 or no children) it is linked from the parent (or the first +0x144 for a negative n; the old head's +0x50 not set, sic), else after that many steps, or the last; its own
// +0x50 not set, sic. The answer 1 plus the steps. The bar and the redraw as Window705950_InsertSorted.
u16 __fastcall Window705950_InsertAt(u8 *window, void *, s32 n, s32 position, const u16 *text, u32 value);

// 0x65b66a (cdecl (node)): the node, its next ones and all their children freed (Mem_Free, each after its children); how many.
s32 __cdecl Window705950_FreeNodes(u8 *node);

// 0x65b486 (thiscall (n), pops 4): 0 when n is not a row (or there is no first node). The n-th node unlinked and freed with its children, the count less them; a selection of n cleared (-1); with [0xfaa5d4]
// and n among the rows shown the window posted 3; the bar (child 1) shown with the range count - rows..0 when more rows than fit, else hidden with 0..0. The answer 1.
u16 __fastcall Window705950_RemoveNode(u8 *window, void *, s32 n);

// 0x65b6d6 (thiscall): all the nodes freed, the count 0, the selection -1, no first node; with [0xfaa5d4] the window posted 3; the bar (child 1) hidden with the range 0..0.
void __fastcall Window705950_Clear(u8 *window);

// 0x65b79c (thiscall (n, text), pops 8): the n-th node's text (0x20 units, CRT_wcsncpy); 1, or 0 with no such node.
u16 __fastcall Window705950_SetNodeText(u8 *window, void *, s32 n, const u16 *text);

// 0x65b7db (thiscall (n, out), pops 8): the n-th node's text copied out (CRT_wcscpy); 1, or 0 with no such node.
u16 __fastcall Window705950_GetNodeText(u8 *window, void *, s32 n, u16 *out);
}
