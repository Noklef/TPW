#include "panel7cc258.h"

#include "panelparts.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc258_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0xc075, 0x20);
    text(window(self), 0xc071, 0x21);
    text(window(self), 0xc072, 0x22);
    text(window(self), 0xc073, 0x23);
    text(window(self), 0xc074, 0x24);
    text(window(self), 0xc07a, 0x25);
    text(window(self), 0xc07b, 0x26);
    textNone(window(self), 0xc070);
    part(window(self), TextPart6ff3b0_Ctor, 0xc076, 0);
    part(window(self), TextPart6ff3b0_Ctor, 0xc077, 0);
    part(window(self), TextPart6ff3b0_Ctor, 0xc078, 0);
    bar(window(self), kRideStatBar, 0xc079, 0x32);
    part(window(self), TextPart6ff3b0_Ctor, 0xc07d, 0);
    bar(window(self), kHappyGrad, 0xc07c, 0x64);
}
}
