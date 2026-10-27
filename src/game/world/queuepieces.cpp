#include "queuepieces.h"

#include <cstdint>

namespace {

const QueuePiecesEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, const u32 *, u32, u32, u32, u32, u32, s32, u32)>((uintptr_t)0x00463060),
                                 reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)0x00463c80)};
const QueuePiecesEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void QueuePieces_SetEnv(const QueuePiecesEnv *env) { g_env = env ? env : &kGameEnv; }
}
