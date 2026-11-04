// A coaster's first cars placed: reimplementation of 0x435570.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// (Test seam: the fncmp driver's table replaces the calls by address.)
void StartCars_SetCalls(const AddrCall *table, u32 count);

// 0x435570 (cdecl; name invented, unconfirmed): on the coaster [0x790fec]: bit 1 at +0x3c. A type (+4) with flag 0x20 has one car: its part the +0x2c of +0x18 of +0x100 of the model (+0x54) and its position by Object_GetPosition of that part's object in the holder at the model's +0x70. Else two: the parts of key 1 and 2 in the holder at +0x10 (mask 0x400000),
// the position that object's +0x30. No object: zeroes. No part (-1): the position stays. Each: CoasterLink_AddEntry(0, 0, 0x13, 0, 0, the position), CoasterLink_AddCar(it, its +0x1c, +0x24, +0x20, 0x144 for the one else 4, 0, 0, 0, 0). Then +0 bits 0x1001; with no [0x785a2c] tools 0x10; without the argument FrontEnd_EnsureMode; Coaster_SetState(2).
// The original's Debug_Printf is left out.
extern "C" {
void __cdecl Coaster_AddStartCars(u32 keepFrontEnd);
}
