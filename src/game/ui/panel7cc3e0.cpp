#include "panel7cc3e0.h"

#include <cstdint>

#include "../world/ridesweep.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc3e0_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc3d8)) < 2) {
        Window_SetShown(Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x520a), nullptr, 0);
        Window_SetShown(Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x520b), nullptr, 0);
    }
    HoverLabel_Bind(self, nullptr, 0x5212);
}

void __fastcall Panel7cc3e0_Dress(u8 *self) {
    if (!window(self)) return;
    textIfAny(window(self), 0x5211, 0x37);
    partIfAny(window(self), TextPart6ff350_Ctor, 0x5210, 0);
    textIfAny(window(self), 0x520f, 0x38);
    partIfAny(window(self), TextPart6ff3b0_Ctor, 0x520e, 0);
}
}
