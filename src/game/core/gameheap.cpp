#include "gameheap.h"

#include <cstdint>

#include "../files/file.h"

namespace {

const GameHeapEnv kGameEnv = {reinterpret_cast<void *(__cdecl *)(u32)>((uintptr_t)0x0067acc0),
                              reinterpret_cast<void(__cdecl *)(void *)>((uintptr_t)0x0067acb0)};
const GameHeapEnv *g_env = &kGameEnv;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

void GameHeap_SetEnv(const GameHeapEnv *env) { g_env = env ? env : &kGameEnv; }

void *GameHeap_Alloc(u32 size) { return g_env->alloc(size); }

void GameHeap_Free(void *block) { g_env->free(block); }

extern "C" {

u32 __cdecl Mem_AllocLogged(u32 size) { return size ? (u32)(uintptr_t)Mem_GlobalAlloc(size) : 0; }

u32 __cdecl Sound_Alloc(u32 size) {
    if (!size)
        return 0;
    global<u32>(0x00802bd8) += 1;
    return (u32)(uintptr_t)Mem_GlobalAlloc(size);
}

u32 __cdecl Sound_Free(void *block) {
    if (!block)
        return 0;
    global<u32>(0x00802bd8) -= 1;
    return (u32)(uintptr_t)Mem_GlobalFree(block);
}

u32 __cdecl Mem_AllocChecked(u32 size) { return size ? (u32)(uintptr_t)Mem_GlobalAlloc(size) : 0; }

u32 __cdecl Mem_FreeChecked(void *block) { return block ? (u32)(uintptr_t)Mem_GlobalFree(block) : 0; }

}  // extern "C"
