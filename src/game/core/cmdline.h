// The game's command line: reimplementation of its parser (0x40f000).
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorder): CRT_strdup.
void CommandLine_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x405630 (thiscall; name invented, unconfirmed): the object made from GetCommandLineA: a copy (CRT_strdup) at +0x204, cut at each " /" (the space cleared), a pointer to each part from +4 and their
// count at +0: 0x80 parts at most, the last with the rest of the line. Returns the object. Debug_Printf left out.
u8 *__fastcall CommandLine_Split(u8 *self);

// 0x40f000 (cdecl (argc, argv); name unconfirmed): each argument after the first in turn. A switch ('/' or '-' then, any case,
// "v" or "version") stops it with 0; "quickload", "nodebug", "noload" and "SAVEDEBUG" are passed over, "flmouse" sets 0x78826c
// and "bwcursor" 0x788270 (to 1). Anything else (other switches too) is the park file: the first one whose end matches the
// extension at [0x7888c8] (any case) is copied to 0x788164 and 0x788160 set; a second, or one without the extension, is an error
// (GameError_Set(0x785de8, 2, 0, it)) that ends it. Returns 1 when there is no error (0x785de8 is 0), else 0.
u32 __cdecl CommandLine_Parse(s32 argc, char **argv);
}

#include "gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gCmdline_788270, 0x00788270, 4);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0xc], gCmdline_748708, 0x00748708, 0xc);
GAME_DATA(char[0x8], gCmdline_748714, 0x00748714, 0x8);
GAME_DATA(char[0xc], gCmdline_74871c, 0x0074871c, 0xc);
GAME_DATA(char[0x8], gCmdline_748728, 0x00748728, 0x8);
GAME_DATA(char[0x8], gCmdline_748730, 0x00748730, 0x8);
GAME_DATA(char[0xc], gCmdline_748738, 0x00748738, 0xc);
GAME_DATA(char[0x4], gCmdline_74874c, 0x0074874c, 0x4);
