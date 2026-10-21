#include "panel7cc218.h"

#include "panelparts.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc218_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x3e20, 0x11);
    text(window(self), 0x3e1a, 0x12);
    text(window(self), 0x3e1c, 0x13);
    text(window(self), 0x3e1e, 0x14);
    text(window(self), 0x3e1f, 0x15);
    text(window(self), 0x3e1d, 0x16);
    text(window(self), 0x3e22, 0x17);
    part(window(self), TextPart6ff350_Ctor, 0x3e21, 0);
    textNone(window(self), 0x3e1b);
    bar(window(self), kRideStatBar, 0x3e16, 0);
    bar(window(self), kRideStatBar, 0x3e18, 0);
    bar(window(self), kRideStatBar, 0x3e19, 0);
    bar(window(self), kRideStatBar, 0x3e17, 0);
    part(window(self), TextPart6ff3b0_Ctor, 0x3e23, 0);
}

void __fastcall Panel7cc218_DressTip(u8 *self) { panelparts::tip(self); }
}
