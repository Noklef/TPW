#include "queuepieces.h"

#include <cstdint>

#include "../coaster/registry.h"

namespace {

const QueuePiecesEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, const u32 *, u32, u32, u32, u32, u32, s32, u32)>((uintptr_t)0x00463060),
                                 reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)0x00463c80)};
const QueuePiecesEnv *g_env = &kGameEnv;

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
}
