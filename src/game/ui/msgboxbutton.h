// The message box's (the dialog [0x7ca098]'s) buttons given their look by kind: reimplementation of 0x48e610. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x48e610 (cdecl (window, kind); the message box's set-up at 0x48e38f / 0x48e3b8, the kinds its dwords 3 and 4): the object of the GUI
// window registered under the kind's name (GuiWindows_FindByName) as the window's +0x98 (Window_SetObject98), its slot 5 called with 1
// and +0x90 the kind's tag: 3 / 4 "mb_locate" (0x204 / 0x203), 9-17 "mb_buy", 18 "b_loan", 19 "mb_resrch", 20 "b_pkinfo",
// 21 "b_staffcost", 23 "mb_callmech" (each 0x200 + the kind), 24 "b_entry" (0xec), 26 "mb_accept" and 27 "mb_decline" (+0x90 left as
// it is). Kind 22 leaves the window alone; any other its +0x98 none and its slot 5 called with 0.
void __cdecl MsgBox_SetButton(u8 *window, u32 kind);
}
