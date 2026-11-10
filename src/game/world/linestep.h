// Setting up a walk between two cells: reimplementation of 0x511350.
//
// The arguments are two cell positions, the column in the low byte and the row in the next. The
// walk's globals: 0x7cf3a8 the column distance, 0x7cf3ac the row distance, 0x7cf3c4 whether the
// column distance is the greater (1) or not (0), 0x7cf398 half of each distance added up, and the
// directions 0x7cf3b0 (the column's: 1 for an increase, else 3) and 0x7cf3b4 (the row's: 2 for an
// increase, else 0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x511350 (cdecl; from, to): fills the globals above. With the same column the column direction
// is the row's; with the same row the row direction is the column's (already the row's when the
// columns were also the same).
void __cdecl LineStep_Init(u32 from, u32 to);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, gLineStep_Sum_7cf398, 0x007cf398, 4);
GAME_DATA(s32, gLineStep_AbsX_7cf3a8, 0x007cf3a8, 4);
GAME_DATA(s32, gLineStep_AbsY_7cf3ac, 0x007cf3ac, 4);
GAME_DATA(u32, gLineStep_ColumnDir_7cf3b0, 0x007cf3b0, 4);
GAME_DATA(u32, gLineStep_RowDir_7cf3b4, 0x007cf3b4, 4);
GAME_DATA(u32, gLineStep_XMajor_7cf3c4, 0x007cf3c4, 4);
