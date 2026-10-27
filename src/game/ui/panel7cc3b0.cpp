#include "panel7cc3b0.h"

#include <cstdint>

#include "guirect.h"
#include "panelparts.h"
#include "thingpreview.h"
#include "window.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc3b0_ShowPreview(u8 *self) {
    if (!*reinterpret_cast<u8 *const *>((uintptr_t)0x007c2658)) return;
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const shown = *reinterpret_cast<u8 **>(self + 0x18);
    Window_GetSavedRect(shown, nullptr, rect);
    ThingPreview_Show(rect, *reinterpret_cast<u8 *const *>((uintptr_t)0x007c2658), *reinterpret_cast<const s16 *>(shown + 0xe0) + 1, 0x41000000);
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc3b0_Dress(u8 *self) {
    if (!window(self)) return;
    text(window(self), 0x51a, 0x40);
    text(window(self), 0x51d, 0x41);
    text(window(self), 0x519, 0x43);
    text(window(self), 0x518, 0x45);
    text(window(self), 0x51c, 0x44);
    text(window(self), 0x516, 0x42);
    textNone(window(self), 0x51b);
    bar(window(self), kHappyGrad, 0x513, 0x64);
    bar(window(self), kRideStatBar, 0x514, 0x64);
    part(window(self), TextPart6ff3b0_Ctor, 0x51e, 0);
    bar(window(self), kRideStatBar, 0x515, 0x1f4);
    textNone(window(self), 0x517);
}
}
