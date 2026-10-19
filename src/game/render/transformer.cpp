#include "transformer.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __fastcall Transformer_Transform(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f) {
    using Slot = u32(__cdecl *)(void *, void *, u32, u32, u32, u32, u32, u32);
    void *const transformer = global<void *>(0x0087b1d8);
    void *const *const vtable = *static_cast<void *const *const *>(transformer);
    return reinterpret_cast<Slot>(vtable[1])(transformer, object, a, b, c, d, f, e);
}

}
