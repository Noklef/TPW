// The game panel object at 0x7cc218 (its vtable 0x6ffbe8). Reimplementation of 0x4ad720, 0x4ad890. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4ad890 (thiscall; slot 3 of the vtable 0x6ffbe8): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x3e20 text 0x11, 0x3e1a text 0x12, 0x3e1c text 0x13, 0x3e1e text 0x14, 0x3e1f text 0x15, 0x3e1d text 0x16, 0x3e22 text 0x17, 0x3e21 a
// 0x6ff350 part with 0, 0x3e1b no text, 0x3e16 a ridestatbar bar with 0, 0x3e18 a ridestatbar bar with 0, 0x3e19 a ridestatbar bar with 0,
// 0x3e17 a ridestatbar bar with 0, 0x3e23 a 0x6ff3b0 part with 0.
void __fastcall Panel7cc218_Dress(u8 *self);

// 0x4ad720 (thiscall; slot 4 of the vtable 0x6ffbe8): the window +0x1c dressed as panelparts::tip (the same code in four panels).
void __fastcall Panel7cc218_DressTip(u8 *self);
}
