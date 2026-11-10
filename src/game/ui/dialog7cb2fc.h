// The dialog window at 0x7cb2fc (its rows of on / off settings: the callers 0x4a36c0 and 0x4a4160 pass the rows 0x1d4cb, 0x1d4cc, ...
// with the label texts 0x145, 0x146, ...). Reimplementation of 0x4a3960, 0x4a43b0 and 0x4a4490. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a3960 (cdecl (row, label, on: a byte, other: a byte, select: a byte, button)): the dialog's child `row`'s child 0x1d4c9 (Window_FindChild
// twice, neither checked) given its slot 9 the label's text with the text 0x14c + on (0x14e + on with `other`) after it: the latter
// (Language_WideText) copied to a 32-unit buffer on the stack by wcsncpy with the count 31 (its last unit never set: a text of 31 units
// or more leaves it unterminated), then the label's widened into the language's wide buffer 0x7ba058 (whatever Language_WideText returns,
// that buffer is what is used: with no label it still holds the first text) and the copy appended there (wcscat). With `select`, the
// dialog's 0x705a50 child `button` (not checked) then gets the state !on, notifying (Window705a50_SetState(.., 1)).
void __cdecl Dialog7cb2fc_SetToggle(u32 row, u32 label, u8 on, u8 other, u8 select, u32 button);

// 0x4a33e0 (cdecl (row, label, value); name invented, unconfirmed): the dialog's child `row`'s child 0x1d4c9 (neither checked) given its slot 9 the label's text with, after it, the value formatted (0x7537b8) or with a negative value the text 0x14e (0x7537c8), on a stack buffer of 10 units; a value not negative also the row's slot 7.
void __cdecl Dialog7cb2fc_SetValueRow(u32 row, u32 label, s32 value);

// 0x4a43b0 (cdecl (row)): the dialog's child `row`'s child 0x1d4c9 (Window_FindChild for each step, neither checked) dressed: a
// TextPart_Ctor part (0x1a4 bytes, GameHeap_Alloc; none when that fails), the bytes 0, 0, 0, 0xff as 0x40000, the font of the dword
// [0x7537b4] (Language_Font) and the text 0.
void __cdecl Dialog7cb2fc_DressRow(u32 row);

// 0x4a4490 (cdecl (row, value)): as Dialog7cb2fc_DressRow, then the row (a 0x705b20 slider) given the range 0-100
// (Window705b20_SetRange) and its slot 7 the value.
void __cdecl Dialog7cb2fc_DressSlider(u32 row, u32 value);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gDialog7cb2fc_7537b4, 0x007537b4, 0x4);
GAME_DATA(u32[2], gDialog7cb2fc_7537c8, 0x007537c8, 0x8);
