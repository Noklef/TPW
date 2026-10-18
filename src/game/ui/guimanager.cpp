#include "guimanager.h"

#include <cstdint>

#include "../core/accessors.h"

namespace {

u8 *manager() { return *reinterpret_cast<u8 **>((uintptr_t)0x0077c48c); }

u32 callSlot(u32 slot) {
    u8 *const m = manager();
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *)>((*reinterpret_cast<void ***>(m))[slot])(m, nullptr);
}
}

extern "C" {

u32 __cdecl GuiManager_CallSlot4() { return callSlot(4); }

u32 __cdecl GuiManager_CallSlot3() { return callSlot(3); }

u32 __cdecl GuiManager_Word4Set() { return Fn_659790(manager()) != 0 ? 1 : 0; }
}
