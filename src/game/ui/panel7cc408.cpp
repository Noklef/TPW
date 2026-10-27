#include "panel7cc408.h"

#include <cstdint>

#include "../core/soundplay.h"
#include "../world/advisor.h"
#include "dialogs.h"
#include "guirect.h"
#include "panelparts.h"
#include "thingpreview.h"
#include "window.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc408_ShowPreview(u8 *self) {
    if (!*reinterpret_cast<u8 *const *>((uintptr_t)0x007c2658)) return;
    u8 rect[0xc];
    GuiRect_Ctor(rect);
    u8 *const shown = *reinterpret_cast<u8 **>(self + 0x18);
    Window_GetSavedRect(shown, nullptr, rect);
    ThingPreview_Show(rect, *reinterpret_cast<u8 *const *>((uintptr_t)0x007c2658), *reinterpret_cast<const s16 *>(shown + 0xe0) + 2, 0x41400000);
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc408_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc430) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007575e0), 0x004b7720, 0x3722, 0x3723, 0);
    Sound_PlayAt(0, *reinterpret_cast<const u32 *>((uintptr_t)0x00803a24), 0x81, 0, 0, 0);
    Advisor_PostId(0x12e);
}

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
