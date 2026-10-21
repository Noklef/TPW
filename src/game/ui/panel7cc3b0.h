// The game panel object at 0x7cc3b0 (its vtable 0x6ffe50). Reimplementation of 0x4b53c0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b53c0 (thiscall; slot 3 of the vtable 0x6ffe50): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7): 0x51a
// text 0x40, 0x51d text 0x41, 0x519 text 0x43, 0x518 text 0x45, 0x51c text 0x44, 0x516 text 0x42, 0x51b no text, 0x513 a happygrad bar
// with 0x64, 0x514 a ridestatbar bar with 0x64, 0x51e a 0x6ff3b0 part with 0, 0x515 a ridestatbar bar with 0x1f4, 0x517 no text.
void __fastcall Panel7cc3b0_Dress(u8 *self);
}
