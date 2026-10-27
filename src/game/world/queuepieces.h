// The queue pieces: rides made from the table at 0x763388 (12-byte rows: +0 the ride type, +4 the model's name (quedead.md2, ...),
// the byte +8 a flag; 0x522900 loads them, 0x5229b0 destroys them), placed at a cell with a key of 0x33a. Reimplementation of 0x5229e0 and
// 0x522a90 (cdecl). Ride_Create (0x463060) and Ride_Destroy (0x463c80) are agent B's and not yet ported: the env calls them.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the queue pieces call in the game, so that they can run outside it (the benches point these at recorders).
struct QueuePiecesEnv {
    u32(__cdecl *create)(u32 type, u32 key, const u32 *desc, u32 flag, u32, u32, u32, u32, s32, u32);   // 0x463060
    void(__cdecl *destroy)(u32 ride);                                                                    // 0x463c80
};
// The queue pieces' calls go to env (nullptr: the game's).
void QueuePieces_SetEnv(const QueuePiecesEnv *env);

// 0x5229e0 (cdecl (x, y, unused, piece, angle, claim); called by 0x536682 / 0x536730; name invented): a description on the stack {x, y,
// 360 - angle (0 for an angle of 0), 0x816d68}; with claim, Ride_Claim(0, the piece's type, 0x33a, it, the piece's flag byte, 0, 0, 0),
// else the env's create (0x463060: the type, 0x33a, it, the flag byte, 0, 0, 0, 0, -1, 0). Returns that call's result.
u32 __cdecl QueuePiece_Place(u32 x, u32 y, u32 unused, u32 piece, u32 angle, u32 claim);

// 0x522a90 (cdecl (ride); name invented): the env's destroy (0x463c80) of the ride.
void __cdecl QueuePiece_Remove(u32 ride);
}
