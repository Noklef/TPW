// The game's WinMain: reimplementation of 0x45a960. Name invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorders): every callee but the strings, the heap and the imports.
void GameMain_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x45a960 (stdcall (instance, previous, command line, show), pops 0x10): with Startup_CheckCanRun's AL 1: the arguments to the
// globals, the folders, language and balance files set, the user slots ([0xf81ebc]) and online object ([0xf7d88c]) dropped and the
// slots made anew, the startup options and "save\config.tcf" loaded, the wheel's message registered; then the message loop until bit 2
// of [0x7a1a14] (or GetMessage's 0). Each pass with bit 1 of it: until bit 4 the language, disc, folders, screen, fonts, window and
// game started (a failure sets bit 2), the online object made if none and polled, the frame run and its answer ORed in.
// Afterwards the sound, screen and game shut down; the slots made if none; unless their +0x60 is -1 they are finished (0x5c8650) and
// the chat client [0x7cddec] dropped; the engine shut down, the error shown, the fonts freed. Returns the last message's wParam
// (the original's slot is unset when no message was asked for; here 0).
u32 __stdcall Game_WinMain(u32 instance, u32 previous, u32 commandLine, u32 show);
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x10], gGamemain_74d11c, 0x0074d11c, 0x10);
GAME_DATA(char[0x10], gGamemain_74d12c, 0x0074d12c, 0x10);
