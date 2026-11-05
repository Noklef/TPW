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

// 0x584b20 (cdecl; its result in ST0): the angle from (a1, a2) to (a3, a4), 0 to 2 pi: the acos of
// their dot product over their lengths, taken from 2 pi when the asin of their cross product is
// below 0 or a NaN. Name invented.
double __cdecl LightList_Angle(float a1, float a2, float a3, float a4);

// 0x584be0 (thiscall, RET 0xc): adds a light by its direction: 0 at 4 or more lights, else 1. The
// direction, normalised, to +0x170 + 12 i; its entry to +0x40 + 0x40 i; its cross product with the
// global vector at 0x768ab0 to +0x140 + 12 i. With a dot product times -127 above 0, that value's
// low byte, shifted to the top, to +0x1b0 + 4 i, the angle to +0x1a0 + 4 i, and the count goes up;
// otherwise the count stays. The angle's scale comes from x and y but scales x and z (sic). Name invented.
u32 __fastcall LightList_Add(u8 *list, void *, float x, float y, float z);
}
