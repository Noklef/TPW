// The game panel object at 0x7ca178 (its vtable 0x6ff740). Reimplementation of 0x497100. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x497100 (thiscall; slot 3 of the vtable 0x6ff740): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x15bbe text 0x3c, 0x15bbc text 0x3d, 0x15bbf text 0x3f, 0x15bbd a 0x6ff350 part with 0, 0x15bbb a ridestatbar bar with 0x32, 0x15bc0 a
// 0x6ff3b0 part with 0.
void __fastcall Panel7ca178_Dress(u8 *self);
}
