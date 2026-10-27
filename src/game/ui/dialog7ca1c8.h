// The dialog whose window is at 0x7ca1c8 (built by 0x498790 from the template 0x751530; 0x7ca1cc set while it is up): its nine check buttons
// (ids 0x18-0x21 in its child 0x18) shown from a flags word, or one of them selected. Reimplementation of 0x498090, 0x498a50, 0x498a80,
// 0x4981d0, 0x498a30, 0x4819f0, 0x4987f0 and 0x498790.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x498090 (cdecl (flags); name unconfirmed): in the child 0x18 of the window [0x7ca1c8] (Window_FindChild), slot 5 (bool) of the child
// 0x1f gets bit 0 of flags, 0x20 bit 1 when the byte +0x50 of [0x7ca1d0] is also not 0, 0x1b bit 2, 0x19 bit 3, 0x1a bit 4, 0x1c bit 5,
// 0x21 bit 6, 0x1e bit 7 and 0x1d bit 8. A missing child faults.
void __cdecl Dialog7ca1c8_SetChecks(u32 flags);

// 0x498a50 (cdecl (flags); name unconfirmed): Dialog7ca1c8_SetChecks when [0x7ca1c8] and [0x7ca1cc] are both not 0.
void __cdecl Dialog7ca1c8_SetChecksIfUp(u32 flags);

// 0x481a00 (thiscall (flags), pops 4; name unconfirmed; `this` unused): Dialog7ca1c8_SetChecksIfUp(flags).
void __fastcall Dialog7ca1c8_SetChecksIfUpThis(void *self, void *, u32 flags);

// 0x498a80 (cdecl; name unconfirmed): the window [0x7ca1c8] takes the focus (Window_SetFocus(0), its result returned).
u32 __cdecl Dialog7ca1c8_Focus();

// 0x4981d0 (cdecl (flags); name unconfirmed): in the child 0x18 of the window [0x7ca1c8] (Window_FindChild; a ui/radiogroup), the first
// flag of 4, 1, 2, 8, 0x10, 0x20, 0x40, 0x80, 0x100 that is set selects the button 0x1b, 0x1f, 0x20, 0x19, 0x1a, 0x1c, 0x21, 0x1e, 0x1d
// (RadioGroup_Select); those from 8 on also set the game cursor (GameCursor_Set) 0xe, 0xc, 0xd, 0xa, 0xf, 0x10. No flag: none selected (-1).
void __cdecl Dialog7ca1c8_SelectButton(u32 flags);

// 0x498a30 (cdecl (flags); name unconfirmed): Dialog7ca1c8_SelectButton when [0x7ca1c8] and [0x7ca1cc] are both not 0.
void __cdecl Dialog7ca1c8_SelectButtonIfUp(u32 flags);

// 0x4819f0 (thiscall on the object at 0x7b51f0, this not used (flags), pops 4; name unconfirmed): Dialog7ca1c8_SelectButtonIfUp(flags).
void __fastcall Dialog7ca1c8_SelectButtonIfUpThis(void *self, void *, u32 flags);

// 0x4987f0 (cdecl; name unconfirmed): the dialog's button 0x20 (in its child 0x18; Window_FindChild): when the byte +0x50 of [0x7ca1d0]
// is 0, no object +0x98 (Window_SetObject98) and its slot 5 called with 0; else, by the coaster edit tools on offer (CoasterEdit_Tools),
// its slot 5 called with 1, the object of the GUI window registered as "cb_trough" (tool 0x200; +0x90 and +0x94 0x134) or "cb_loop"
// (0x132) as its +0x98, then slot 5 called again with whether tool 2 is on offer.
void __cdecl Dialog7ca1c8_UpdateLoopButton();

// 0x498790 (cdecl; name invented): the dialog built on the GUI root [0xfaa5d8] from the template 0x751530 with the handler 0x4982a0
// (WindowScript_Build) as [0x7ca1c8], taken out of the root's children (Window_Unlink), its dword +0xa0 index 0 set to 2
// (Window_SetDwordA0) and hidden (Window_SetShown(0)); none of it checked.
void __cdecl Dialog7ca1c8_Create();
}
