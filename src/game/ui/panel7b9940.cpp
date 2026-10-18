#include "panel7b9940.h"

#include <cstdint>

#include "../render/renderer.h"
#include "guimanager.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
}

extern "C" {

u32 __cdecl Panel7b9940_Detach() {
    GuiManager_CallSlot4();
    const u8 *const table = global<const u8 *>(0x007b9940);
    const u32 base = 0x007b9940 + *reinterpret_cast<const u32 *>(table + 4);
    return Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, base, 0x20);
}
}
