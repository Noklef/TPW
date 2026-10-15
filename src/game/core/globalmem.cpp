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
}
