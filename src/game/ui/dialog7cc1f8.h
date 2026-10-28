// The dialog window at 0x7cc1f8 (its handler's switch calls the dresser on message 0x15). Reimplementation of 0x4ab6b0, 0x4abfb0 and 0x4abc80. The names are
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

// 0x4abc80 (cdecl (item: its id word); called by 0x4ac478; name invented): the help tip of a build list item under the mouse. With the
// mouse (Gui_GetMousePos) outside the dialog's child 0x1f8's saved rectangle (Window_FindChild, not checked; Window_GetSavedRect into
// a GuiRect) the tip state [0x75513c] -1. Else for a negative id other than the one tipped (the word [0x7cc1e0]): the state 4, the id
// tipped and the help background given (HelpBg_SetText) the text 0x21a for -1, 0x21b for -2 (the table [0x78ba98], 0x80 characters;
// for another the stack buffer as it is, sic). For an object id: the state 0 / 1 when its definition's +0xc4 is set and the global
// save lacks the key (GlobalSave_HasKey): 0 unlocked (ObjectDef_IsUnlocked), 1 not; else 3 when its control record's cost +4
// (ObjectControls_Find on the game [0x7cf83c]'s +0x1d943c, quiet; read first) is at most the money record's balance
// (Source_Field0c of GameTable7cfb90_Get1da726_5195d0), 2 when not. With the state or the id new both stored and the tip given the text
// 0x97 / 0x98, or 0x96 / 0x94 with the definition's name (its text +0x90 of the table [0x78bad0], 0x3f characters) as the string
// argument 0.
void __cdecl Dialog7cc1f8_ShowItemTip(const s16 *item);
}
