// The row windows the GUI library's list controls (0x6639cb) are given factories and initialisers for: reimplementation of 0x480d30,
// 0x480dd0, 0x4a8380, 0x4a8420, 0x4b4c70, 0x4bacc0 and 0x4bad70.
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x480d30 (cdecl, no arguments; the row factory of the "hilight" list built at 0x480b00): a window of 0x138 bytes (GameHeap_Alloc;
// 0 when that fails) built (Window_Ctor: no parent, flags 0, id 0, the rectangle (0, 0, 100, height)) with the height font 7's
// (Language_Font; its slot 1, a signed word) * 0x600 / the signed word [0xfaa5c0] + 6. The height is worked out before the allocation.
u8 *__cdecl HilightList_NewRow();

// 0x4a8380 (cdecl, no arguments; the row factory of the list of the window [0x7cc188]): the same, the height worked out only once the
// allocation succeeded.
u8 *__cdecl List7cc188_NewRow();

// 0x480dd0 / 0x4a8420 (cdecl (row); the row initialisers of the same two lists, the same code): the row's impl a new text part (0x1a4
// bytes, GameHeap_Alloc; TextPart_Ctor(0), or none when the allocation fails; Window_SetImpl), font 7 (Language_Font; Window_Set4c) and
// the four bytes 0xff (Window_Notify40000).
void __cdecl HilightList_InitRow(u8 *row);
void __cdecl List7cc188_InitRow(u8 *row);

// 0x4b4c70 (cdecl, no arguments; the row factory of the list built by 0x4b4a50, the child 0x1e7b of the window [0x7cc338]): the height
// worked out once (the dword [0x7cc35c] while 0: the font [0x756f38]'s height as above), then a window of 0x138 bytes built as above
// with the height's low word, and when there is one a child of it (Window_CreateChild: kind 9, flags 0x11, id 1, the rectangle
// (100, 0, 200, 50)). Returns the window or 0.
u8 *__cdecl List7cc338_NewRow();

// 0x4bacc0 (cdecl, no arguments; the row factory of the list child 0xc of the window [0x7cc530], built at 0x4ba980): as 0x480d30, then
// a child of the window (kind 6, flags 1, id 2, the same rectangle) whether the window was made or not (sic: 0 faults).
u8 *__cdecl List7cc530_NewRow();

// 0x4bad70 (cdecl (row); that list's row initialiser): as 0x480dd0, then the row's first child (+0x5c) given the object of the GUI
// window registered as "i_boxtick" (GuiWindows_FindByName; Window_SetObject98) and its slot 7 called with 0.
void __cdecl List7cc530_InitRow(u8 *row);
}
