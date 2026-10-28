#include "queuepieces.h"

#include <cstdint>

#include "../coaster/registry.h"
#include "../motion/animator.h"
#include "rides.h"

namespace {

const QueuePiecesEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, const u32 *, u32, u32, u32, u32, u32, s32, u32)>((uintptr_t)0x00463060),
                                 &Ride_Destroy};
const QueuePiecesEnv *g_env = &kGameEnv;

// A piece of the table at `table` (12-byte rows: the ride type, the flag byte +8) placed at (x, y) moved by the turn.
u32 place_turned(u32 table, s32 x, s32 y, u32 piece, u32 angle, u32 claim) {
    u32 turn = 360 - angle;
    if (turn == 360) turn = 0;
    switch (turn) {
    case 90: ++y; break;
    case 180: ++x; ++y; break;
    case 270: ++x; break;
    default: break;
    }
    const u32 desc[4] = {static_cast<u32>(x), static_cast<u32>(y), turn, 0x008179f8};
    const u8 *const row = reinterpret_cast<const u8 *>((uintptr_t)(table + piece * 12));
    const u32 type = *reinterpret_cast<const u32 *>(row);
    const u32 flag = row[8];
    if (claim) return Ride_Claim(0, type, 0x33a, desc, flag, 0, 0, 0);
    return g_env->create(type, 0x33a, desc, flag, 0, 0, 0, 0, -1, 0);
}

}  // namespace

extern "C" {

void QueuePieces_SetEnv(const QueuePiecesEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __cdecl QueuePiece_Place(u32 x, u32 y, u32, u32 piece, u32 angle, u32 claim) {
    u32 turn = 360 - angle;
    if (turn == 360) turn = 0;
    const u32 desc[4] = {x, y, turn, 0x00816d68};
    const u8 *const row = reinterpret_cast<const u8 *>((uintptr_t)(0x00763388 + piece * 12));
    const u32 type = *reinterpret_cast<const u32 *>(row);
    const u32 flag = row[8];
    if (claim) return Ride_Claim(0, type, 0x33a, desc, flag, 0, 0, 0);
    return g_env->create(type, 0x33a, desc, flag, 0, 0, 0, 0, -1, 0);
}

void __cdecl QueuePiece_Remove(u32 ride) { g_env->destroy(ride); }

u32 __cdecl TrackQueuePiece_Place(s32 x, s32 y, u32, u32 piece, u32 angle, u32 claim) {
    return place_turned(0x007633e8, x, y, piece, angle, claim);
}

u32 __cdecl TrackQueuePiece_PlaceAnimated(s32 x, s32 y, u32, u32 piece, u32 angle, u32 claim) {
    const u32 ride = place_turned(0x007634d8, x, y, piece, angle, claim);
    if (ride != 0) Animator_Play(*reinterpret_cast<u8 **>((uintptr_t)(0x007a4610 + ride * 4)), 0xc, 0, 7, 1.0f, 0);
    return ride;
}

u32 __cdecl EndPiece_Place(s32 x, s32 y, u32, u32 piece, u32 angle, u32 claim) {
    return place_turned(0x00763608, x, y, piece, angle, claim);
}
}
