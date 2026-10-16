#include "gameheap.h"

#include <cstdint>

namespace {

const GameHeapEnv kGameEnv = {reinterpret_cast<void *(__cdecl *)(u32)>((uintptr_t)0x0067acc0),
                              reinterpret_cast<void(__cdecl *)(void *)>((uintptr_t)0x0067acb0)};
const GameHeapEnv *g_env = &kGameEnv;

}  // namespace

void GameHeap_SetEnv(const GameHeapEnv *env) { g_env = env ? env : &kGameEnv; }

void *GameHeap_Alloc(u32 size) { return g_env->alloc(size); }

void GameHeap_Free(void *block) { g_env->free(block); }
