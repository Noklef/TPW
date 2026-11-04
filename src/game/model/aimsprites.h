// A model's sprite vertices turned to the camera: each picks the cell of its sprite sheet (8 turns, 7 tilts) nearest the direction it is seen from.
#pragma once

#include "../core/addrcalls.h"
#include "../../../re/types.h"

extern "C" {

// Its calls to the sprite scripts go by the game's addresses; a table of replacements (tests/fncmp/aimsprites.py gives them recorders).
void AimSprites_SetCalls(const AddrCall *table, u32 count);

// 0x44b510 (cdecl (instance); name invented, unconfirmed): with bit 4 of the holder's (instance+0) vertex block (+0x28), for each of the model's (+4) vertices (u16 +0x48; 0x14 bytes from block+4) with bit 2: its face (model+0x46 on from the first,
// counted as in the parts' flags) with 0x400 negates the first three of the vertex's matrix's (+4) axes X and Z; with 0x8000 only register 13 of its script slot (+0xc) goes to -1. Otherwise SetPosition from the matrix's translation; the matrix's inverse, times the camera
// (0x790a98), gives a direction. Of 8 turns of -45 degrees about Y in each of 7 tilts of 30 degrees about X from (0, 1, 0), the one with the greatest positive dot product wins (tilts 0 and 6: turn 0), register 13 = turn + tilt * 8; a NaN X direction: turn and tilt 0.
// A second matrix from that up vector and the (0, 0, -1) turned and tilted the same way: register 9 its roll over 2 pi, register 8 the float 0x74ced0. sic: a direction with no winner keeps the turn, tilt and up vector of the vertex before (the stack's on the first).
void __cdecl AimSprites_Update(const u8 *instance);
}
