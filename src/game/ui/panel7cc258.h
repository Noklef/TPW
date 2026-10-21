// The game panel object at 0x7cc258 (its vtable 0x6ffc68). Reimplementation of 0x4afc10. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4afc10 (thiscall; slot 3 of the vtable 0x6ffc68): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0xc075 text 0x20, 0xc071 text 0x21, 0xc072 text 0x22, 0xc073 text 0x23, 0xc074 text 0x24, 0xc07a text 0x25, 0xc07b text 0x26, 0xc070 no
// text, 0xc076 a 0x6ff3b0 part with 0, 0xc077 a 0x6ff3b0 part with 0, 0xc078 a 0x6ff3b0 part with 0, 0xc079 a ridestatbar bar with 0x32,
// 0xc07d a 0x6ff3b0 part with 0, 0xc07c a happygrad bar with 0x64.
void __fastcall Panel7cc258_Dress(u8 *self);
}
