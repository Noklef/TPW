// The game's progress dialog (the window [0x7cca94], built from the script 0x759548: a "progress.wct" bar and its text child 0x2241). Its
// Window_SendMessage wrapper (0x4c25e0) is in dialogs. Reimplementation of 0x4c23c0, 0x4c23f0, 0x4c25b0 and 0x4c2600. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4c23c0 (cdecl (window, message, a, b); name unconfirmed): the dialog's handler. Message 0x14 forgets the dialog ([0x7cca94] = 0);
// every message goes on to Window_CallDefault (its result returned).
s32 __cdecl Dialog7cca94_OnMessage(u8 *window, u32 message, u32 a, u32 b);

// 0x4c23f0 (cdecl (text); name unconfirmed): with the dialog up, its child 0x2241 (not checked) given the text through its slot 9.
// Otherwise the dialog built (WindowScript_Build of the root [0xfaa5d8], the script 0x759548, the handler 0x4c23c0) into [0x7cca94]
// and dressed: a bar of "progress.wct" (0x124 bytes, MeshOverlay_Construct; none when the allocation fails) as its implementation, its
// slot 7 given 0; the child 0x2241 (looked up for each step, not checked) a TextPart_Ctor part, 1, 1 as 0xc00 and 0x3000, font 6, the
// bytes 0, 0, 0, 0xff as 0x40000 and the text; the dialog's depth 0x23 and its slot 5 given 0. Each step reads [0x7cca94] again.
void __cdecl Dialog7cca94_Show(const u16 *text);

// 0x4c25b0 (cdecl (index); name unconfirmed): unless the dialog is up with this index already shown ([0x7cca9c]), Dialog7cca94_Show of
// the language text `index`; [0x7cca9c] = index.
void __cdecl Dialog7cca94_ShowText(u32 index);

// 0x4c2600 (cdecl (percent); name unconfirmed): with the dialog up, its slot 7 given (percent << 10) / 100 (signed, rounded towards 0).
void __cdecl Dialog7cca94_SetProgress(u32 percent);
}
