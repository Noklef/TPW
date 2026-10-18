#include "globalmem.h"

#include <cstdint>

namespace {

template <typename T> T slot(u32 address) { return *reinterpret_cast<const T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void *__cdecl GlobalMem_Alloc(u32 size, u32 zero) {
    if (!size) return nullptr;
    return slot<void *(__stdcall *)(u32, u32)>(0x006fd1cc)((u8)zero & 1 ? 0x40 : 0, size);  // GMEM_ZEROINIT
}

void *__cdecl GlobalMem_Free(void *block) {
    if (!block) return block;
    return slot<void *(__stdcall *)(void *)>(0x006fd26c)(block);
}

void *__cdecl GlobalMem_AllocCounted(u32 count) {
    if (!count) return nullptr;
    u32 *block = static_cast<u32 *>(GlobalMem_Alloc(0x40, 1));
    block[0] = 0;
    block[1] = count;
    return block;
}

void *__cdecl GlobalMem_FreeAtB0(const u8 *owner) {
    return GlobalMem_Free(*reinterpret_cast<void *const *>(owner + 0xb0));
}

void __cdecl GlobalMem_FreeThree_427bb0() {
    static const u32 kSlots[3] = {0x0074c8d4, 0x0074c8c0, 0x0074c8b8};
    for (const u32 slot : kSlots) {
        void *&block = *reinterpret_cast<void **>((uintptr_t)slot);
        GlobalMem_Free(block);
        block = nullptr;
    }
}

void __cdecl GlobalMem_InitUnusedFlag() { *reinterpret_cast<u32 *>(0x007a1c60) = 1; }

void __cdecl GlobalMem_InitUnusedOne() { *reinterpret_cast<float *>(0x007a1c64) = 1.0f; }

void __cdecl GlobalMem_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007a1c68) = 1.0f / 8388608.0f; }
}
