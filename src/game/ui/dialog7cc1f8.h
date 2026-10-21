// The dialog window at 0x7cc1f8 (its handler's switch calls the dresser on message 0x15). Reimplementation of 0x4ab6b0 and 0x4abfb0. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4ab6b0 (no arguments): the dialog's child 0x1ed (Window_FindChild; nothing when it has none) gets its children dressed in this order
// (ui/panelparts; a text a TextPart_Ctor part with that language text, a bar's value given to the child's slot 7): 0x1f4 text 0x7d, 0x1f3
// text 0x7e, 0x1f1 text 0x7f, 0x1f6 text 0x80, 0x1f7 text 0x81, 0x1f5 text 0x88, 0x1f2 text 0x88 (sic, the same text), 0x1ee a
// ridestatbar bar with 0x1f4, 0x1ef one with 0xc8, 0x1f0 one with 0x64.
void __cdecl Dialog7cc1f8_DressStats();

// 0x4abfb0 (cdecl (mode)): the dialog's child 0x1ed (nothing when it has none, the mode looked at after) gets some of those children
// shown or hidden (Window_FindChild, Window_SetShown with 1 or 0; a missing one skipped), in the order 0x1f3, 0x1f1, 0x1f6, 0x1f7, 0x1ee,
// 0x1f2, 0x1ef, 0x1f0: mode 0 all shown; 1 and 3 all hidden; 2 0x1f3 and 0x1ee shown, the others hidden (0x1f6 twice, sic); others nothing.
void __cdecl Dialog7cc1f8_ShowStats(u32 mode);
}
