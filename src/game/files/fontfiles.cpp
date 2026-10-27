// The font files the game adds for its run: see fontfiles.h.
#include "fontfiles.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vcall.h"

namespace {
u8 *&Global(u32 address) { return *reinterpret_cast<u8 **>(static_cast<std::uintptr_t>(address)); }
}  // namespace

extern "C" {

void __cdecl FontFiles_Destroy() {
    u8 *const end = Global(0x00789c20);
    for (u8 *name = Global(0x00789c1c); name != end; name += 0x14)
        vcall<void>(name, 0xc, 0u);
    GameHeap_Free(Global(0x00789c1c));
    Global(0x00789c1c) = nullptr;
    Global(0x00789c20) = nullptr;
    Global(0x00789c24) = nullptr;
}
}  // extern "C"
