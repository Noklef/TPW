#include "texman.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall TexManager_SetLoadFlags(u8 *manager, void *, u32 flags) { at<u32>(manager, 0x10008) = flags; }
}
