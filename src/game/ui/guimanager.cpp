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

u32 __cdecl GuiManager_CallSlot4Or3() { return callSlot(Fn_659790(manager()) != 0 ? 4 : 3); }

u32 __fastcall Obj7b51f0_GuiManagerSlotAndFlag(void *) {
    GuiManager_CallSlot4Or3();
    const u32 set = GuiManager_Word4Set();
    *reinterpret_cast<u8 *>((uintptr_t)0x0078d90e) = (u8)set;
    return set;
}

u32 __cdecl GuiClock_Now() {
    u8 *const clock = *reinterpret_cast<u8 **>((uintptr_t)0x0077c490);
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *)>((*reinterpret_cast<void ***>(clock))[0])(clock, nullptr);
}

void __cdecl Gui_GetMousePos(u16 *x, u16 *y) {
    *x = *reinterpret_cast<const u16 *>((uintptr_t)0x00faa540);
    *y = *reinterpret_cast<const u16 *>((uintptr_t)0x00faa544);
}
}
