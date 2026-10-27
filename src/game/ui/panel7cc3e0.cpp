#include "panel7cc3e0.h"

#include "panelparts.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc3e0_Dress(u8 *self) {
    if (!window(self)) return;
    textIfAny(window(self), 0x5211, 0x37);
    partIfAny(window(self), TextPart6ff350_Ctor, 0x5210, 0);
    textIfAny(window(self), 0x520f, 0x38);
    partIfAny(window(self), TextPart6ff3b0_Ctor, 0x520e, 0);
}
}
