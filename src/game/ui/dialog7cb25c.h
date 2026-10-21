// The dialog at 0x7cb25c (its mode flag at 0x7cb260). Reimplementation of 0x49f860. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x49f860 (cdecl): with the dialog [0x7cb25c] (nothing without it), its child 0x12c4bd's children 0x11, 0x12 and 0x15 (Window_FindChild,
// not checked) each sent a language text as 0x100 to their impl (Language_WideText; Window_NotifyImpl100) and given a value at +0x90 and
// +0x94: texts 0xae / 0xad / 0xb1 and values 0xdf / 0xe0 / 0xe3 when [0x7cb260] is 0, else texts 0xb2 / 0xb3 / 0xb4 and values 0xe5 / 0xe6 /
// 0xe7; then the dialog's child 0x12c4ca told the text 0xaa (0xab with the flag) through its slot 9.
void __cdecl Dialog7cb25c_SetTexts();

}
