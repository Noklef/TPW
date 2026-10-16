// The renderer's light list: reimplementation of 0x584b10, 0x584e30 and 0x584e50.
//
// The list at 0x8c1658 is cleared each frame and, with renderer state flag 0x800, given each of
// the scene's type 1 lights by its direction (0x584be0, at most 4): the direction, normalised,
// goes to +0x170 + 12 i and a 0x40-byte entry built from it to +0x40 + 0x40 i. +0x1c0 is the
// count, +0x1c4 the cursor the sprite drawer (0x589858, 0x589a71) walks them with.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x584b10 (thiscall): count and cursor = 0.
void __fastcall LightList_Clear(u8 *list);

// 0x584e30 (thiscall): cursor = 0; the first entry (+0x40), or null when the count is not above 0.
u8 *__fastcall LightList_First(u8 *list);

// 0x584e50 (thiscall): the cursor moved on; the entry there (+0x40 + 0x40 cursor), or null once
// it reaches the count (signed).
u8 *__fastcall LightList_Next(u8 *list);
}
