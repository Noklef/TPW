// The dialog at 0x7cb25c (its mode flag at 0x7cb260: 0 the loans offered, 1 those taken). Reimplementation of 0x49f860, 0x49fa00 and
// 0x4a0770. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x49f860 (cdecl): with the dialog [0x7cb25c] (nothing without it), its child 0x12c4bd's children 0x11, 0x12 and 0x15 (Window_FindChild,
// not checked) each sent a language text as 0x100 to their impl (Language_WideText; Window_NotifyImpl100) and given a value at +0x90 and
// +0x94: texts 0xae / 0xad / 0xb1 and values 0xdf / 0xe0 / 0xe3 when [0x7cb260] is 0, else texts 0xb2 / 0xb3 / 0xb4 and values 0xe5 / 0xe6 /
// 0xe7; then the dialog's child 0x12c4ca told the text 0xaa (0xab with the flag) through its slot 9.
void __cdecl Dialog7cb25c_SetTexts();

// 0x49fa00 (cdecl (list); name invented): the list (a Window7059f0) cleared, then for each of the eight loans of the world's money record
// (the table 0x7cfb90's entry for [0x7cf83c]'s word +0x1da726; Money_LoanOffer, which marks the offer +0 by the park's credit): with
// [0x7cb260] 0, an offer not taken (+0x14 0) within the credit (+0 set) added with the data i + 1; with it 1, a loan taken added with the data
// -1 - i. The six columns: its text (Records_DataIfTerminated of [0x78bac4] for +0x1c, Language_Widen), the amount +4, the months left
// (the term +0xc, less the months paid +0x18 for a loan taken), +8, the rate +0x10 and the rate times the months left (Window7059f0_AddItem,
// parent -1, refreshed).
void __cdecl Dialog7cb25c_FillList(u8 *list);

// 0x4a0770 (cdecl): with the dialog and [0x7cb260] 1, Dialog7cb25c_FillList on its child 0x12c4bd (Window_FindChild).
void __cdecl Dialog7cb25c_Refresh();

// 0x4a06b0 (cdecl (list, message, a, b); name invented): the list's handler (set at 0x4a0121). On 0x101 with b 1 (a 0x15 test first, of
// no effect): [0x7cb260] = whether a is 0x12c4bf (the button pressed), the list refilled (Dialog7cb25c_FillList) and the texts set
// (Dialog7cb25c_SetTexts). Every message then goes to Window_CallDefault, whose answer is returned.
s32 __cdecl Dialog7cb25c_ListProc(u8 *list, u32 message, u32 a, u32 b);

}
