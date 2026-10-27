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
}
