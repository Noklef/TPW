// The dialog window at 0x7cb2fc (its rows of on / off settings: the callers 0x4a36c0 and 0x4a4160 pass the rows 0x1d4cb, 0x1d4cc, ...
// with the label texts 0x145, 0x146, ...). Reimplementation of 0x4a3960. The names are unconfirmed.
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
}
