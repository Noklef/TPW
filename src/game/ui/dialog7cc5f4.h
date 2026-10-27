// The two-field entry dialog (its window [0x7cc5f4] when opened by Dialog7cc5f4_Create). Reimplementation of 0x4bb070, 0x4ba200 and 0x4bb390. The
// names are unconfirmed.
// Built by Dialog_Create from a script: its children 0x49 and 0x4b two entry fields (Window705a20) on the buffers 0x7cc590 and 0x7cc548,
// each given a new text part as its implementation (TextPart6ff4d0 / TextPart6ff380; none when the allocation fails), font 7, the
// colours (0xff, 0xff, 0xff, 0xff) (Window_Notify40000), Window_NotifyC00(0, 0), the "hilight" window as its caret and its text (slot 9);
// the labels 0x4a / 0x4c inside them a new TextPart, font 7 and a language text each; the field 0x49 takes the focus; the child 0x4d a
// TextPart6ff530, font 5, Window_NotifyC00(1, 1), a language text (slot 9) and the yellow (HoverLabel_SetYellow). The original checks
// none of the windows.
#pragma once

#include "../../../re/types.h"

// 0x4bb070 (cdecl (owner)): the dialog of the script 0x7585a8 with the handler 0x4bb030, made [0x7cc5f4]; [0x7cc5fc] = owner; the
// fields' buffers 0x10 bytes, their texts 0x78a374, the labels' texts 0xed / 0xee, 0x4d's 0xec. The field 0x4b and the child 0x4d are
// looked up from [0x7cc5f4] read again.
void __cdecl Dialog7cc5f4_Create(u32 owner);

// 0x4bb390 (cdecl (value); name invented): [0x7cc5f4] sent message 4 (0, 0) (Window_SendMessage); then, with value non-zero and the owner
// [0x7cc5fc] set, the owner called with value (the original's tail jump) and its result returned; else 0.
u32 __cdecl Dialog7cc5f4_Finish(u32 value);

// 0x4ba200 (cdecl (value, data)): as Dialog7cc5f4_Create with the script 0x7583e8 and the handler 0x4b9d00, the dialog not kept (nor
// the owner); the fields' buffers 0x20 bytes, their texts data + 0x234 / data + 0x436, the labels' texts 0xea / 0xeb, 0x4d's 0xe9; then
// the dialog's dword 0 +0xa0 = value (Window_SetDwordA0) and KeyMaps_ResetAllAndSetFlag.
void __cdecl Dialog7cc5f4_CreateFrom(u32 value, u8 *data);
