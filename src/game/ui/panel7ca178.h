// The game panel object at 0x7ca178 (its vtable 0x6ff740). Reimplementation of 0x496f90, 0x497100, 0x497990. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x497100 (thiscall; slot 3 of the vtable 0x6ff740): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x15bbe text 0x3c, 0x15bbc text 0x3d, 0x15bbf text 0x3f, 0x15bbd a 0x6ff350 part with 0, 0x15bbb a ridestatbar bar with 0x32, 0x15bc0 a
// 0x6ff3b0 part with 0.
void __fastcall Panel7ca178_Dress(u8 *self);

// 0x496f90 (thiscall; slot 4 of the vtable 0x6ff740): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7ca178_DressTip(u8 *self);

// 0x497990 (thiscall (thing), pops 4; slot 10 of the vtable 0x6ff740): the selected thing [0x7ca19c] = thing, the window built
// (WindowAt10_Create: the script 0x751190, the handler 0x497640, the children 0x15bb8, 0x15bba and 0x15bb9), and the advisor posted 0xd5
// when the low byte of the thing's float +0x44 (__ftol) is under 25, else 0x12b (Advisor_PostId).
void __fastcall Panel7ca178_Show(u8 *self, void *, u8 *thing);
}
