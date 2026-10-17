#include "gameheap.h"

#include <cstdint>

#include "../files/file.h"

namespace {

const GameHeapEnv kGameEnv = {reinterpret_cast<void *(__cdecl *)(u32)>((uintptr_t)0x0067acc0),
                              reinterpret_cast<void(__cdecl *)(void *)>((uintptr_t)0x0067acb0)};
const GameHeapEnv *g_env = &kGameEnv;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

constexpr u32 kTrackedCount = 0x008bb214, kTrackedPairs = 0x0087b1f8;  // TrackedGlobal_Alloc / _Free

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

u32 __cdecl TrackedGlobal_Alloc(u32 size) {
    if (global<u32>(kTrackedCount) == 0x8000)
        return 0;
    const u32 block = (u32)(uintptr_t)(*Files_GetEnv()->globalAlloc)(0x40, size);  // GMEM_ZEROINIT
    if (!block)
        return 0;
    const u32 n = global<u32>(kTrackedCount);
    global<u32>(kTrackedPairs + n * 8) = block;
    global<u32>(kTrackedPairs + n * 8 + 4) = block;
    global<u32>(kTrackedCount) = n + 1;
    return block;
}

u32 __cdecl TrackedGlobal_Free(u32 block) {
    const s32 last = (s32)(global<u32>(kTrackedCount) - 1);
    s32 i = last;
    while (i >= 0 && global<u32>(kTrackedPairs + (u32)i * 8 + 4) != block)
        --i;
    if (i == -1) {  // (a count of -1 or below starts below -1: that index is taken, sic)
        i = last;
        while (i >= 0 && global<u32>(kTrackedPairs + (u32)i * 8 + 4) != block + 4)
            --i;
        if (i == -1)
            return 0;
    }
    (*Files_GetEnv()->globalFree)(reinterpret_cast<void *>((uintptr_t)global<u32>(kTrackedPairs + (u32)i * 8)));
    const u32 n = global<u32>(kTrackedCount);
    if (i < (s32)(n - 1)) {
        global<u32>(kTrackedPairs + (u32)i * 8) = global<u32>(kTrackedPairs + (n - 1) * 8);
        global<u32>(kTrackedPairs + (u32)i * 8 + 4) = global<u32>(kTrackedPairs + (n - 1) * 8 + 4);
    }
    global<u32>(kTrackedCount) = n - 1;
    return 1;
}

u32 __fastcall TrackedArray_Alloc(u8 *array, void *, u32 count) {
    const u32 block = TrackedGlobal_Alloc(count * 4);
    at<u32>(array, 0x14) = block;
    if (!block)
        return 0;
    at<u32>(array, 8) = count;
    return block;
}

u32 __fastcall TrackedArray_Free(u8 *array) {
    if (at<u32>(array, 0x14)) {
        TrackedGlobal_Free(at<u32>(array, 0x14));
        at<u32>(array, 0x14) = 0;
    }
    at<u32>(array, 8) = 0;
    return 1;
}

void __fastcall TrackedBlock_Dtor(u8 *holder) {
    if (at<u32>(holder, 0)) {
        TrackedGlobal_Free(at<u32>(holder, 0));
        at<u32>(holder, 0) = 0;
    }
}

u32 __fastcall TrackedBlock_Free(u8 *holder) {
    TrackedBlock_Dtor(holder);
    return 1;
}

}  // extern "C"
