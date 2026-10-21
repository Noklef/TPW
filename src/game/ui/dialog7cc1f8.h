// The dialog window at 0x7cc1f8 (its handler's switch calls the dresser on message 0x15). Reimplementation of 0x4ab6b0. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4ab6b0 (no arguments): the dialog's child 0x1ed (Window_FindChild; nothing when it has none) gets its children dressed in this order
// (ui/panelparts; a text a TextPart_Ctor part with that language text, a bar's value given to the child's slot 7): 0x1f4 text 0x7d, 0x1f3
// text 0x7e, 0x1f1 text 0x7f, 0x1f6 text 0x80, 0x1f7 text 0x81, 0x1f5 text 0x88, 0x1f2 text 0x88 (sic, the same text), 0x1ee a
// ridestatbar bar with 0x1f4, 0x1ef one with 0xc8, 0x1f0 one with 0x64.
void __cdecl Dialog7cc1f8_DressStats();
}
