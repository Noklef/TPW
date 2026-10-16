#include "meshqueue.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall MeshQueue_Clear(u8 *queue) { at<u32>(queue, 0x400) = 0; }

void __cdecl MeshQueue2_Init() { global<u32>(0x008bb638) = 0; }

u32 __fastcall MeshQueue2_Add(u32 *queue, void *, u32 mesh) {
    s32 n = (s32)queue[0];
    if (n >= 0x100)
        return 0;
    queue[n + 1] = mesh;
    queue[0] = queue[0] + 1;
    return 1;
}
}
