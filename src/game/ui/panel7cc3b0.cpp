#include "panel7cc3b0.h"

#include <cstdint>

#include "../world/advisor.h"
#include "../world/camera.h"
#include "../world/ridesweep.h"
#include "dialogs.h"
#include "guirect.h"
#include "hoverlabel.h"
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

void __fastcall Panel7cc3b0_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc3a4)) < 2) {
        Window_SetShown(Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x50d), nullptr, 0);
        Window_SetShown(Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x50f), nullptr, 0);
    }
    HoverLabel_Bind(self, nullptr, 0x520);
}

void __fastcall Panel7cc3b0_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc3a4) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00756f48), 0x004b5cb0, 0x511, 0x512, 0);
    Window_SetHandler(*reinterpret_cast<u8 **>(self + 0x18), nullptr, 0x004b5c80);
    Advisor_PostId(0xc0);
}

s32 __cdecl Panel7cc3b0_WindowHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x10006) Camera_CentreOnGlobalThing();
    return Window_CallDefault(window, nullptr, message, a, b);
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
