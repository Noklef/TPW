#include "panel7ca178.h"

#include <cstdint>

#include "../world/advisor.h"
#include "dialogs.h"
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

void __fastcall Panel7ca178_DressTip(u8 *self) { panelparts::tip(self); }

void __fastcall Panel7ca178_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007ca19c) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00751190), 0x00497640, 0x15bb8, 0x15bba, 0x15bb9);
    const u8 value = (u8)(u32)(long long)*reinterpret_cast<const float *>(thing + 0x44);   // (__ftol's low byte)
    Advisor_PostId(value < 0x19 ? 0xd5 : 0x12b);
}
}
