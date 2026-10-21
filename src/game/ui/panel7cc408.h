// The game panel object at 0x7cc408 (its vtable 0x6fff40). Reimplementation of 0x4b6c20. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b6c20 (thiscall; slot 3 of the vtable 0x6fff40): with the window +0x14 set (read again for each child), its children dressed in this
// order (ui/panelparts; a text a TextPart_Ctor part with that language text, a part's or a bar's value given to the child's slot 7):
// 0x372c text 0x46, 0x3725 text 0x47, 0x3726 text 0x48, 0x3727 text 0x4b, 0x3728 text 0x49, 0x3729 text 0x4a, 0x372a text 0x4c, 0x372b
// text 0x4d, 0x3724 a 0x6ff3b0 part with 0x29a, 0x372d a 0x6ff4a0 part (no slot 7), 0x372e a 0x6ff350 part with 0, 0x372f a 0x6ff350 part
// with 0, 0x3730 a 0x6ff350 part with 0, 0x3731 a 0x6ff350 part with 0, 0x3732 (when found) a happygrad bar (no slot 7), 0x3733 no text.
void __fastcall Panel7cc408_Dress(u8 *self);
}
