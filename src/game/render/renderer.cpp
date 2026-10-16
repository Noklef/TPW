#include "renderer.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Renderer_SetRestoreCallback(u8 *renderer, void *, u32 fn) { at<u32>(renderer, 0x78) = fn; }

void __fastcall Renderer_SetStateFlags(u8 *renderer, void *, u32 flags) { at<u32>(renderer, 0x48) = flags; }

u32 __fastcall Renderer_StateFlags(const u8 *renderer) { return at<u32>(renderer, 0x48); }
}
