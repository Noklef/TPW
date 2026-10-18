// The row windows the GUI library's list controls (0x6639cb) are given factories for: reimplementation of 0x480d30 and 0x4a8380.
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
}
