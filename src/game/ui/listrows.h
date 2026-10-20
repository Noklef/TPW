// The row windows the GUI library's list controls (0x6639cb) are given factories and initialisers for: reimplementation of 0x480d30,
// 0x480dd0, 0x4a8380 and 0x4a8420.
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
}
