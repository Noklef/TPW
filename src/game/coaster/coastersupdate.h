// All coasters updated for a frame.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

// Its calls go by the game's addresses; a table of replacements (tests/fncmp/coastersupdate.py gives them recorders).
void CoastersUpdate_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x435a30 (cdecl; name invented, unconfirmed): down the coasters' list (0x790fe0, next at +0x18): one with flag 1 made current; the selected (0x790fec) with flag 4 places its pending piece; one without flag 0x2000 logs its id (ActionRec_WriteW 0x43, u16 +0x14); each rebuilds and takes the flag.
// Then each with a bit of 0xe at +8 runs its trains; with its type's flag 4 its textures scroll by the time since the last (+0x138) times +0x2c and 1/1000: each of the type's count (+0x20) offsets (+0x13c) advanced, past 1 losing its whole part, which the step for the following loses too (sic); and down the car list (+0x38, next +0x38) the V of each vertex used by a material with flag 0x20 (of up to 0x100, sic).
// With a bit of 0xc at +8 and no 0x40: each car (+0xf4 of 0x80 bytes at +0x128) with flag 2 at the station, and each its sounds. 0x792218 and 0x792220 end 0.
void __cdecl Coasters_Update();
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8 *, gCoastersupdate_792218, 0x00792218, 0x4);
GAME_DATA(u32, gCoasters_Update_792220, 0x00792220, 0x4);
