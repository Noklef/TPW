#include "panel7cc218.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../gfx/barmesh.h"
#include "../core/accessors.h"
#include "../core/trivial.h"
#include "../render/textparts.h"
#include "../world/advisor.h"
#include "../world/object.h"
#include "../world/objectmsg.h"
#include "../world/ridelist.h"
#include "../world/ridesweep.h"
#include "dialogs.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"
#include "window705a50.h"
#include "window705b20.h"

using namespace panelparts;

namespace {
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
u8 *root(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x10); }
s32 mode(const u8 *self) { return *reinterpret_cast<const s32 *>(self + 8); }

// The child `id` of the panel's window +0x10 (not checked): the range 0..0x78, and its child 0x3e2e a "smallguage.wct" mesh (0x124
// bytes; none when the allocation fails) as its implementation and its slot 7 given 0.
void gauge(u8 *self, u32 id) {
    u8 *const child = Window_FindChild(root(self), nullptr, id);
    Window705b20_SetRange(child, nullptr, 0, 0x78);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x124));
    u8 *const mesh = memory ? MeshOverlay_Construct(memory, nullptr, reinterpret_cast<const char *>(0x007558f4)) : nullptr;
    Window_SetImpl(Window_FindChild(child, nullptr, 0x3e2e), nullptr, mesh);
    vcall<void>(Window_FindChild(child, nullptr, 0x3e2e), 0x1c, 0u);
}

// The child `id` of the panel's window +0x10 (not checked): a TextPart_Ctor part (0x1a4 bytes; none when the allocation fails), the
// bytes 1, 1 as 0xc00, font 6, the bytes 0, 0, 0, 0xff as 0x40000 and no text.
void label(u8 *self, u32 id) {
    u8 *const child = Window_FindChild(root(self), nullptr, id);
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(child, nullptr, 1, 1);
    Window_Set4c(child, nullptr, Language_Font(6));
    Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
    Window_NotifyImpl100(child, nullptr, 0);
}
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

void __fastcall Panel7cc218_SetUpGauges(u8 *self) {
    u8 rect[12];
    GuiRect_Ctor(rect);
    if (root(self)) {
        gauge(self, 0x3e2d);
        gauge(self, 0x3e30);
        Window705b20_SetRange(Window_FindChild(root(self), nullptr, 0x3e2f), nullptr, 0, 10);
        label(self, 0x3e31);
        label(self, 0x3e33);
        label(self, 0x3e32);
    }
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc218_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc208) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00755150), 0x004af600, 0x3e24, 0x3e15, 0x3e25);
    at<u32>(self, 0x24) = 0;
    const u8 value = (u8)(u32)(long long)at<float>(thing, 0x44);   // (__ftol's low byte)
    Advisor_PostId(value <= 0x14 && value != 0 ? 0x129 : 0xcf);
}

void __fastcall Panel7cc218_ApplyToShown(u8 *self) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    Obj_SetSpeed(thing, nullptr, at<u32>(self, 0x2c));
    Object_SetCapacity(thing, nullptr, at<u8>(self, 0x30));
    if (at<u32>(Object_GetDef(thing), 0x70)) Obj_SetDuration(thing, nullptr, at<u8>(self, 0x34));
}

void __fastcall Panel7cc218_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc208)) < 2) {
        Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e26), nullptr, 0);
        Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e27), nullptr, 0);
    }
    Panel7cc218_SetUpGauges(self);
    Window_SetShown(root(self), nullptr, 1);
    HoverLabel_Bind(self, nullptr, 0x3e28);
}

void __fastcall Panel7cc218_UpdateButtons(u8 *self) {
    u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007c2658);
    if (!thing) return;
    const u8 *const def = Object_GetDef(thing);
    if (mode(self) == 3) {
        static const u32 kButtons[] = {0x3e38, 0x3e34, 0x3e2a, 0x3e36, 0x3e2c, 0x3e37, 0x3e35};
        for (u32 i = 0; i < sizeof(kButtons) / sizeof(kButtons[0]); ++i)
            Window_SetShown(Window_FindChild(root(self), nullptr, kButtons[i]), nullptr, 0);
        return;
    }
    u8 *const open = Window_FindChild(root(self), nullptr, 0x3e38);
    Window_SetShown(open, nullptr, 1);
    bool enabled = false;
    if (mode(self) != 0x19 && mode(self) != 0x1a && !(Is68_Zero_4e0440(thing) && !Object_CanOpen(thing)) &&
        (Is68_Zero_4e0440(thing) || Const1_424ec0())) {
        vcall<void>(open, 0x14, 1u);
        Window705a50_SetState(open, nullptr, Is68_Zero_4e0440(thing) ? 1 : 0, 1);
        enabled = true;
    }
    if (!enabled) vcall<void>(open, 0x14, 0u);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e34), nullptr, 1);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e2a), nullptr, at<s32>(def, 0x9c) != 0 ? 1 : 0);
    u8 *const kind = Window_FindChild(root(self), nullptr, 0x3e36);
    Window_SetShown(kind, nullptr, 1);
    const s32 m = mode(self);
    Window705a50_SetState(kind, nullptr, (m == 0x13 || m == 0x15 || m == 0x12 || m == 0x14 || m == 0x11) ? 1 : 0, 1);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e35), nullptr, 1);
    u8 *const track = Window_FindChild(root(self), nullptr, 0x3e2c);
    Window_SetShown(track, nullptr, 1);
    vcall<void>(track, 0x14, (u8)Object_TrackNeighboursOk(thing) ? 1u : 0u);
    Window_SetShown(Window_FindChild(root(self), nullptr, 0x3e37), nullptr, 1);
}
}
