// A park's coasters read from its save (0x437b60). x87 in the original's order.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// Its calls go by the game's addresses; a table of replacements so that it can run outside the game (tests/fncmp/coastersload.py gives every one a recorder).
void CoastersLoad_SetEnv(const AddrCall *table, u32 count);

// 0x437b60 (cdecl (stream); name invented, unconfirmed): always 1 (the 0 it has for a failed read can not come: it tests Stream_Read's result for below 0, and that is 0 or 1; a short read goes on over what the
// stack holds, sic). Dwords: the count of coasters, then 0x790b04, 0x790b00, 0x791ce0. For each coaster:
// - 0x790afc 1 while its pieces are read; a header of 0x20 bytes: flags dword, +4 dword, +0xa id (found in the list at 0x790fe0 by its +0x10; none faults on the null it goes on with), +0xc, +0xe, +0x10 words, +0x12 dword,
//   +0x16 count of pieces, +0x18, +0x1a words, +0x1c dword. The coaster becomes current (0x790fec, 0x790fe4 its +0x3c), CoasterLink_SetUpCars, CoasterLink_AddFixedPieces (the type's float +0x44), IdTable_Add (its answer at +0x14), +0 |= 2 | 0x4000.
// - The first piece's words (a dword count at +0xb0, the words as dwords from +0xb4). Each other piece: 0x22 bytes: flag 8 CoasterLink_BeginEnd; CoasterEdit_PlaceCar answers the piece, or goes to the next one; its length, +0xe0 and angles
//   (Car_Set*At), flag 8's +8 bits from the flags, the piece's words as the first's (flags 0x40 and 0x80: the pieces kept for the flag 2 below).
// - Each train (+0x4c, by +0x28) not flagged: its cars' +8 bit 0x100 for all but the last, their +0x90 and +0x74 from it (and the model's +0x18: flag 8), +0 |= 0x1001 of the coaster, Train_PlaceCars.
// - Flag 1: the coaster's +0x3c |= 1; 1 entry (the type's flag 0x20: at the object or model part named) or 2 (the parts by key 0x400000): CoasterLink_AddEntry, CoasterLink_AddCar; +0 |= 0x1001, CoasterEdit_SetTools (0x10) without 0x785a2c, Coaster_SetState (2).
// - Every piece's words made into pieces (Track_PieceAt). Flag 2: its pieces at +0x60, +0x64, +0x3c |= 2, the first's +8 |= 0x50000 at its kind 2. Flags 4-0x100: +0 bits 0x20-0x800. Then +8, +0x2c, +0xf4, +0xf0, +0x140 from the header.
// - For each of the type's +0x2e8 trains: 0x1b bytes to its record (+0x128, 0x80 each), and for each of its seats 0xb bytes to the record at +0x28 (0x60 each): with a model, Ride_Claim and unless flagged asleep, Animator_SetPlaying; its two buffers.
// - Coaster_SetupCars, CoasterEdit_EndAll, Coaster_FlashCar unless +0x3c has only 1 of 1, 2; 0x790fec 0.
u32 __cdecl Coasters_Load(void *stream);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8 *, gCoastersload_790fe4, 0x00790fe4, 0x4);
GAME_DATA(u8 *, gCoastersload_790fec, 0x00790fec, 0x4);
