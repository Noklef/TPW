// The GUI library's manager object (its pointer at 0x77c48c) and clock (0x77c490): the cdecl front ends that call them. Reimplementation
// of 0x659607, 0x65961c, 0x659631 and 0x65968e.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x659607 / 0x65961c (cdecl; names unconfirmed): slot 4 / slot 3 (thiscall, no arguments) of the manager [0x77c48c]; its result returned.
u32 __cdecl GuiManager_CallSlot4();
u32 __cdecl GuiManager_CallSlot3();

// 0x659631 (cdecl; name unconfirmed): 1 when the manager's word +4 (Fn_659790) is not 0, else 0.
u32 __cdecl GuiManager_Word4Set();

// 0x65968e (cdecl; name unconfirmed): slot 0 (thiscall, no arguments) of the clock [0x77c490]: the time the GUI's timed messages use.
u32 __cdecl GuiClock_Now();
}
