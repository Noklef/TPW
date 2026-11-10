// The game's start and its first movies: reimplementation of 0x54dcf0, 0x54df20 and 0x54e070. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

// Calls by the game's address (the bench's recorders): everything but the strings and the window's import.
void GameStart_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x54dcf0 (cdecl): the clock frozen, the texture folders set (1 for a first run's [0x785874] of 0 or 1, or after the file settings with bit 0x800 of [0x79d86c]), the command line parsed: it failing, the error shown and nothing more. Key maps; a dated
// line for the debugger (made, then dropped); [0x879068] = 1, CRT srand of timeGetTime; sound started, its levels from the options (the camera's halved unless [0x78d8e0]), the start sound; the TV's static; the front end's GUI behind a load screen of text
// "0x764fe4", 0x1f4; state [0x87906c] = 4 with 0x200 in [0x7a1a8c], else 9; [0x875034] = the clock's now, the advisor initialised.
void __cdecl Game_Start();

// 0x54df20 (cdecl): the path "Data\Movies\bf.tgq" put in the buffer 0x8782d0 and looked for in the archives (VFS_ArchiveName, an answer of 1 in AL): found: Display_Restore and the movie played.
void __cdecl Game_PlayLogoMovie();

// 0x54e070 (cdecl): as 0x54df20 with a movie by the day of the month (localtime's +0xc, signed, remainder by 8: outside 0..7 the buffer stays), and the display left as it is. Played at the window's client origin, the height 640 (x * 64 / 480).
void __cdecl Game_PlayDayMovie();
}
