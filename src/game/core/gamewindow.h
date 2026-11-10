// The game's window and the engine's start on it: reimplementation of 0x44e080. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorders): every callee but the imports.
void GameWindow_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x44e080 (cdecl): the window class (handler 0x46bcb0, the arrow cursor in its icon slot, sic) registered and the window made:
// popup style 0x90000000 of the screen's size with bit 0 of the flags [0x7a1a8c], else 0xcf0000 with its rect adjusted for that
// style; its handle to [0x7a130c]. Either failing: error 4 and 0. Shown and updated. With [0x78d968] == 1: the graphics options
// applied (the settings file first unless [0x79d86c] has 0x4000; that failing: 0, no error), else the options' settings loaded.
// Engine_Start from the flags (failing: error 0xc and 0). The matrix at [0x87afe8] made the identity, the perspective at 0x87b110
// set (0.1, 1000, 90). [0x785874] 0-3 gives 3-0 to [0x8c4b44]'s +0x1920 and +0x18f4; the option byte from [0x8bd524] == 1 with its
// flags in [0x79d86c] (0x80 or 0x40; 0x6000 of the flags cleared when not); with [0x78d8e4] == 1 and [0x879a24]'s +0x788 at most 1
// (signed) option C set to 0. 1.
u32 __cdecl GameWindow_Create();
}

#include "gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gGamewindow_7a130c, 0x007a130c, 0x4);
