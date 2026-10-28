#include "panel7cc3e0.h"

#include <cstdint>

#include "../world/advisor.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/ridesweep.h"
#include "../world/world.h"
#include "dialogs.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
}  // namespace

extern "C" {

void __fastcall Panel7cc3e0_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc3d8)) < 2) {
        Window_SetShown(Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x520a), nullptr, 0);
        Window_SetShown(Window_FindChild(*reinterpret_cast<u8 **>(self + 0x10), nullptr, 0x520b), nullptr, 0);
    }
    HoverLabel_Bind(self, nullptr, 0x5212);
}

void __fastcall Panel7cc3e0_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc3d8) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007573c8), 0x004b67f0, 0x520c, 0x520d, 0);
    Advisor_PostId(0x12d);
}

void __fastcall Panel7cc3e0_Dress(u8 *self) {
    if (!window(self)) return;
    textIfAny(window(self), 0x5211, 0x37);
    partIfAny(window(self), TextPart6ff350_Ctor, 0x5210, 0);
    textIfAny(window(self), 0x520f, 0x38);
    partIfAny(window(self), TextPart6ff3b0_Ctor, 0x520e, 0);
}

void __fastcall Panel7cc3e0_ShowValues(u8 *self) {
    using SetValue = void(__fastcall *)(u8 *, void *, u32);
    const u8 *const thing = *reinterpret_cast<const u8 *const *>((uintptr_t)0x007c2658);
    Object_GetDef(thing);
    if (!window(self)) return;
    if (u8 *const child = Window_FindChild(window(self), nullptr, 0x5210)) {
        const SetValue set = (*reinterpret_cast<SetValue const *const *>(child))[7];
        GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 *const *>((uintptr_t)0x007cf83c));   // (its answer only the ECX of the stdcall)
        set(child, nullptr, ObjectControls_GetWord18(*reinterpret_cast<const u16 *>(thing + 0xe)));
    }
    u8 *const child = Window_FindChild(window(self), nullptr, 0x520e);
    const SetValue set = (*reinterpret_cast<SetValue const *const *>(child))[7];
    set(child, nullptr, Object_ScrapRefund(thing));
}
}
