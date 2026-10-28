#include "panel7cc3b0.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/idmaps.h"
#include "../core/language.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../world/advisor.h"
#include "../world/analysercell.h"
#include "../world/camera.h"
#include "../world/mousemode.h"
#include "../world/ridesweep.h"
#include "../world/staff.h"
#include "dialogs.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "thingpreview.h"
#include "window.h"
#include "window705a50.h"

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

void __fastcall Panel7cc3b0_ShowStats(u8 *self) {
    u8 *const staff = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (!window(self)) return;
    u32 index = Mode19c_ToIndex(staff);
    const s32 days = Staff_AgeDays(staff);
    u8 age[0x14];   // (a number argument, as TextArgInt_Ctor builds it inline)
    *reinterpret_cast<u32 *>(age) = 0x006ff170;
    *reinterpret_cast<u32 *>(age + 4) = 0x5641524d;
    *reinterpret_cast<u32 *>(age + 8) = 8;
    *reinterpret_cast<s32 *>(age + 0xc) = days;
    *reinterpret_cast<u32 *>(age + 0x10) = 0;
    TextFormat().format(*reinterpret_cast<void **>((uintptr_t)0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, 0x1bb, age);
    vcall<void>(Window_FindChild(window(self), nullptr, 0x51b), 0x24, 0x007ba058u);   // (none of the children checked)
    u8 *child = Window_FindChild(window(self), nullptr, 0x51e);
    vcall<void>(child, 0x1c, (u32)Staff_GetWage(staff));
    child = Window_FindChild(window(self), nullptr, 0x514);
    vcall<void>(child, 0x1c, (Staff_GradeLevel(staff) << 10) / 100u);
    child = Window_FindChild(window(self), nullptr, 0x51f);
    Window_Notify1(child, nullptr, Index_ToCode485ec0(index));
    child = Window_FindChild(window(self), nullptr, 0x517);
    if (index == 1 && !Staff_HasPatrolRegion(staff)) index = 0;
    Language_SetObjectText(child, index);
    child = Window_FindChild(window(self), nullptr, 0x513);
    vcall<void>(child, 0x1c, (u32)((s32)((u32)(u8)(long long)*reinterpret_cast<const float *>(staff + 0x1f8) << 10) / 100));
    child = Window_FindChild(window(self), nullptr, 0x515);
    vcall<void>(child, 0x1c, (u32)((s32)((u32)(u8)(long long)*reinterpret_cast<const float *>(staff + 0x1fc) << 10) / 100));
}

void __fastcall Panel7cc3b0_Update(u8 *self) {
    u8 *const staff = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (!staff) return;
    vcall<void>(*reinterpret_cast<u8 **>(self + 0x20), 0x24, Staff_GetName(staff));
    vcall<void>(self, 0x18);
    vcall<void>(self, 0x14);
    u8 *const patrol = Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x510);   // (neither child checked)
    if (Staff_HasPatrolRegion(staff)) {
        Advisor_PostId(0xd4);
        Window705a50_SetState(patrol, nullptr, 1, 1);
    } else {
        Window705a50_SetState(patrol, nullptr, 0, 1);
    }
    u8 *const cell = Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x50e);
    vcall<void>(cell, 0x14, AnalyserRecord_ChecksCell(staff) ? 1u : 0u);
}

u32 __fastcall Panel7cc3b0_SetPatrol(u8 *self, void *, u32 on) {
    u8 *const staff = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (!on) {
        Staff_ClearPatrolRegion(staff);
        return 0;
    }
    Global7c2650_Set((u32)(uintptr_t)staff);
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
    u8 *const mode = block ? MouseModeTile_Ctor(block, nullptr, 0x36) : nullptr;
    MouseModes_EnsureHolder();   // (inlined there)
    MouseMode_Set(mode);
    vcall<void>(self, 0x2c);
    Advisor_PostId(0xc1);
    return 1;
}
}
