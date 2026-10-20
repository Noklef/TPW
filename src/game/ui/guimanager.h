// The GUI library's manager object (its pointer at 0x77c48c) and clock (0x77c490): the cdecl front ends that call them; and the mouse
// position's getter. Reimplementation of 0x659607, 0x65961c, 0x659631, 0x6595cd, 0x65968e and 0x65958a,
// and 0x4817f0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x659607 / 0x65961c (cdecl; names unconfirmed): slot 4 / slot 3 (thiscall, no arguments) of the manager [0x77c48c]; its result returned.
u32 __cdecl GuiManager_CallSlot4();
u32 __cdecl GuiManager_CallSlot3();

// 0x659631 (cdecl; name unconfirmed): 1 when the manager's word +4 (Fn_659790) is not 0, else 0.
u32 __cdecl GuiManager_Word4Set();

// 0x6595cd (cdecl; name unconfirmed): slot 4 of the manager when its word +4 (Fn_659790) is not 0, else slot 3 (thiscall, no arguments);
// the result returned.
u32 __cdecl GuiManager_CallSlot4Or3();

// 0x4817f0 (thiscall on the object at 0x7b51f0, this not used; name unconfirmed): GuiManager_CallSlot4Or3, then GuiManager_Word4Set to
// the byte 0x78d90e and returned.
u32 __fastcall Obj7b51f0_GuiManagerSlotAndFlag(void *self);

// 0x65968e (cdecl; name unconfirmed): slot 0 (thiscall, no arguments) of the clock [0x77c490]: the time the GUI's timed messages use.
u32 __cdecl GuiClock_Now();

// 0x65958a (cdecl (x, y); name unconfirmed): the GUI's mouse position, the words 0xfaa540 and 0xfaa544, to *x and *y.
void __cdecl Gui_GetMousePos(u16 *x, u16 *y);
}
