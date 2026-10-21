#include "panel7cc408.h"

#include "panelparts.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc408_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x372c, 0x46);
    text(window(self), 0x3725, 0x47);
    text(window(self), 0x3726, 0x48);
    text(window(self), 0x3727, 0x4b);
    text(window(self), 0x3728, 0x49);
    text(window(self), 0x3729, 0x4a);
    text(window(self), 0x372a, 0x4c);
    text(window(self), 0x372b, 0x4d);
    part(window(self), TextPart6ff3b0_Ctor, 0x3724, 0x29a);
    part(window(self), TextPart6ff4a0_Ctor, 0x372d);
    part(window(self), TextPart6ff350_Ctor, 0x372e, 0);
    part(window(self), TextPart6ff350_Ctor, 0x372f, 0);
    part(window(self), TextPart6ff350_Ctor, 0x3730, 0);
    part(window(self), TextPart6ff350_Ctor, 0x3731, 0);
    barIfAny(window(self), kHappyGrad, 0x3732);
    textNone(window(self), 0x3733);
}
}
