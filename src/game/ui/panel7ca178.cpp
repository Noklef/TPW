#include "panel7ca178.h"

#include "panelparts.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7ca178_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x15bbe, 0x3c);
    text(window(self), 0x15bbc, 0x3d);
    text(window(self), 0x15bbf, 0x3f);
    part(window(self), TextPart6ff350_Ctor, 0x15bbd, 0);
    bar(window(self), kRideStatBar, 0x15bbb, 0x32);
    part(window(self), TextPart6ff3b0_Ctor, 0x15bc0, 0);
}
}
