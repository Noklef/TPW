// The GUI library's start and end: the root window [0xfaa5d8] (also [0xfaa5e0]), the message queues [0xfaa5f0] / [0xfaa5f4], the
// window timers' pool and the timed table [0xfaa5f8]. Reimplementation of 0x6584df and 0x658708. The names are invented (unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6584df (cdecl (unused, capacity, flag)): the root window made (Mem_Alloc 0x138; Window_Ctor with no parent, flags 0xf0000001, id
// -0x29a, edges (0, 0, 0x7ff, 0x5ff); none: 0) into [0xfaa5d8]; 0 when none. Else [0xfaa568] = 1, [0xfaa5e0] the root, [0xfaa608],
// [0xfaa57c], [0xfaa578] and word [0xfaa5bc] 0, byte [0xfaa56c] the flag; then two message queues (Mem_Alloc 0x14, Fn_669700) into
// [0xfaa5f0] and [0xfaa5f4], each given the capacity (MsgQueue_Init); the window timers' pool of 0x100 (WindowTimers_Init); the timed
// table (Mem_Alloc 8, Fn_669c41) into [0xfaa5f8] given 0x40 entries (GuiTimers_Init). A failed allocation or a 0 from either Init (as a
// word) stops it with 0 in AX (what was set stays), else 1. Only AX is set.
u32 __cdecl Gui_Init(u32 unused, u32 capacity, u8 flag);

// 0x658708 (cdecl): 0 in AX without the root [0xfaa5d8]. Else the root deleted (its vtable slot 2 with 1), the queues deleted
// (MsgQueue_Delete with 1) and the timed table (GuiTimers_Delete with 1), each when set; the three pointers 0; the window timers' pool
// freed (WindowTimers_Shutdown), the registered windows (GuiWindows_Clear with 1) and the named objects (GuiNamed_FreeAll) let go; the
// root and [0xfaa5dc] / [0xfaa5e0] 0; 1 in AX. Only AX is set.
u32 __cdecl Gui_Shutdown();
}
