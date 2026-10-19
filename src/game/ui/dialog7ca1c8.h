// The dialog whose window is at 0x7ca1c8 (built by 0x498790 from the template 0x751530; 0x7ca1cc set while it is up): its nine check buttons
// (ids 0x18-0x21 in its child 0x18) shown from a flags word. Reimplementation of 0x498090, 0x498a50 and 0x498a80.
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
}
