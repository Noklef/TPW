// The GUI library's small setters and the cursor's plain calls. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65889c (cdecl; words): 0xfaa540 and 0xfaa544 set.
void __cdecl Gui_SetWords540(u32 a, u32 b);
// 0x6590a5 (cdecl): the list 0xfaa5f0's +0x10 (Fn_669c30), -1 without a list.
u32 __cdecl Gui_List5f0Count();
// 0x6590d0 (cdecl; words): the screen's width 0xfaa5c4 and height 0xfaa5c0 set.
void __cdecl Gui_SetScreenSize(u32 width, u32 height);
// 0x6591d4 (cdecl; a word): 0xfaa600 set.
void __cdecl Gui_SetWord600(u32 value);
// 0x6591e3 (cdecl): the cursor 0xfaa608's slot 3, when there is one.
void __cdecl Gui_CursorSlot3();
// 0x659201 (cdecl): the cursor 0xfaa608's slot 2, when there is one.
void __cdecl Gui_CursorSlot2();
// 0x6595b3 (cdecl): 0x77c48c set to the value, or 0xfaa550 for none.
void __cdecl Gui_SetPointer48c(u32 value);
// 0x659674 (cdecl): 0x77c490 set to the value, or 0xfaa580 for none.
void __cdecl Gui_SetPointer490(u32 value);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x2c], gGuisettings_77c48c, 0x0077c48c, 0x2c);
GAME_DATA(u16, gGuisettings_faa540, 0x00faa540, 0x2);
GAME_DATA(u16, gGuisettings_faa544, 0x00faa544, 0x2);
GAME_DATA(u16, gGui_faa600, 0x00faa600, 0x2);
