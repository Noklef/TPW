// The queue pieces: rides made from the table at 0x763388 (12-byte rows: +0 the ride type, +4 the model's name (quedead.md2, ...),
// the byte +8 a flag; 0x522900 loads them, 0x5229b0 destroys them), placed at a cell with a key of 0x33a. Reimplementation of 0x5229e0 and
// 0x522a90 (cdecl). Ride_Create (0x463060) is agent B's and not yet ported: the env calls it. The env's destroy is 0x463c80, Ride_Destroy
// of world/rides (in the game the port; the benches' driver a recorder).
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

// 0x522b90 / 0x522d50 / 0x522f20 (cdecl (x, y, unused, piece, angle, claim); names invented): as QueuePiece_Place over the tables at
// 0x7633e8 (the track queues: gk_trckQ.md2, ...), 0x7634d8 (likewise) and 0x763608 (ho_end1.md2, ...), the description's last dword
// 0x8179f8, and the position moved by the turn (90: y + 1; 180: x + 1 and y + 1; 270: x + 1). 0x522d50's ride (when not 0) then plays bank
// 0xc, entry 0, mode 7, speed 1, layer 0 (its entry of 0x7a4610, Animator_Play). Each returns the ride.
u32 __cdecl TrackQueuePiece_Place(s32 x, s32 y, u32 unused, u32 piece, u32 angle, u32 claim);
u32 __cdecl TrackQueuePiece_PlaceAnimated(s32 x, s32 y, u32 unused, u32 piece, u32 angle, u32 claim);
u32 __cdecl EndPiece_Place(s32 x, s32 y, u32 unused, u32 piece, u32 angle, u32 claim);

// 0x5229b0 / 0x522b60 / 0x522d20 / 0x522ef0 (cdecl, no arguments; names invented, unconfirmed): the env's destroy (0x463c80) of each row's
// +0 in the tables 0x763388 (8 rows), 0x7633e8 (20), 0x7634d8 (25) and 0x763608 (28) whose name pointer (+4) is set; 0x522b60's also
// not when the name starts with '-'.
void __cdecl QueuePieces_Destroy();
void __cdecl TrackQueuePieces_Destroy();
void __cdecl TrackQueuePiecesAnimated_Destroy();
void __cdecl EndPieces_Destroy();
// 0x52f190 (cdecl, no arguments; called by 0x457d97; name invented, unconfirmed): the four above in that order.
void __cdecl AllPieces_Destroy();
}
