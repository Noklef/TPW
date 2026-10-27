#include "panel7cc258.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "../world/object.h"
#include "../world/ridesweep.h"
#include "../world/things.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window705a50.h"
#include "window705af0.h"
#include "window705b20.h"

using namespace panelparts;

namespace {
u8 *window(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x14); }
u8 *root(u8 *self) { return *reinterpret_cast<u8 **>(self + 0x10); }
using Slot = void(__fastcall *)(u8 *, void *, u32);
u8 *selected() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cc288); }

// Panel7cc258_Open's text child `id` when found: a part made by ctor, the bytes 1, 1 as 0xc00, font 6, 0, 0, 0, 0xff as 0x40000.
u8 *label(u8 *window, PartCtor ctor, u32 id) {
    u8 *const child = Window_FindChild(window, nullptr, id);
    if (!child) return nullptr;
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
    Window_SetImpl(child, nullptr, memory ? ctor(memory, nullptr, 0) : nullptr);
    Window_NotifyC00(child, nullptr, 1, 1);
    Window_Set4c(child, nullptr, Language_Font(6));
    Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
    return child;
}
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

void __fastcall Panel7cc258_DressTip(u8 *self) { panelparts::tip(self); }

void __fastcall Panel7cc258_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, (u32)(uintptr_t)selected()) < 2) {
        Window_SetShown(Window_FindChild(root(self), nullptr, 0xc069), nullptr, 0);
        Window_SetShown(Window_FindChild(root(self), nullptr, 0xc06a), nullptr, 0);
    }
    u8 rect[12];
    GuiRect_Ctor(rect);
    if (u8 *const slider = Window_FindChild(root(self), nullptr, 0xc06b)) Window705b20_SetRange(slider, nullptr, 0, 2);
    if (u8 *const slider = Window_FindChild(root(self), nullptr, 0xc086)) Window705b20_SetRange(slider, nullptr, 0, 100);
    if (u8 *const child = Window_FindChild(root(self), nullptr, 0xc085)) {
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(child, nullptr, memory ? TextPart6ff3b0_Ctor(memory, nullptr, 0) : nullptr);
        const s32 low = 0, high = 500, step = 1;
        Window705af0_SetRange(child, nullptr, &low, &high);
        Window705af0_SetStep(child, nullptr, &step);
        Window_Set4c(child, nullptr, Language_Font(6));
        Window_NotifyC00(child, nullptr, 1, 1);
        Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
    }
    if (u8 *const child = label(root(self), TextPart_Ctor, 0xc07e))
        Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(0x27));
    if (u8 *const child = label(root(self), TextPart_Ctor, 0xc084))
        Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_Widen(Thing_IngredientRecord(selected())));
    if (u8 *const child = label(root(self), TextPart_Ctor, 0xc083))
        Window_NotifyImpl100(child, nullptr, (u32)(uintptr_t)Language_WideText(0x28));
    HoverLabel_Bind(self, nullptr, 0xc06e);
    *reinterpret_cast<u32 *>(self + 0x28) = 0;
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc258_Update(u8 *self) {
    const u8 *const thing = *reinterpret_cast<const u8 *const *>((uintptr_t)0x007c2658);
    void *const *const table = *reinterpret_cast<void *const *const *>(*reinterpret_cast<u8 **>(self + 0x20));
    const u32 value = vcall<u32>(self, 0x34);
    // sic: the object's vtable from before the call, the object read again after it
    reinterpret_cast<Slot>(table[9])(*reinterpret_cast<u8 **>(self + 0x20), nullptr, value);
    vcall<void>(self, 0x18);
    vcall<void>(self, 0x14);
    if (u8 *const button = Window_FindChild(root(self), nullptr, 0xc082))
        Window705a50_SetState(button, nullptr, Is68_Zero_4e0440(thing) ? 1 : 0, 1);
    if (u8 *const slider = Window_FindChild(root(self), nullptr, 0xc06b)) vcall<void>(slider, 0x1c, (u32)Obj_GetByte18c(thing) / 50);
    if (u8 *const slider = Window_FindChild(root(self), nullptr, 0xc086)) vcall<void>(slider, 0x1c, (u32)GetByte198_4e21f0(thing));
    u8 *const slider = Window_FindChild(root(self), nullptr, 0xc086);
    u8 *const label = Window_FindChild(root(self), nullptr, 0xc084);
    if (u16 *const wide = Language_Widen(Thing_IngredientRecord(const_cast<u8 *>(thing)))) {
        Window_SetShown(slider, nullptr, 1);
        Window_SetShown(label, nullptr, 1);
        Window_NotifyImpl100(label, nullptr, (u32)(uintptr_t)wide);
    } else {
        Window_SetShown(slider, nullptr, 0);
        Window_SetShown(label, nullptr, 0);
    }
    if (u8 *const figure = Window_FindChild(root(self), nullptr, 0xc085)) vcall<void>(figure, 0x1c, Obj_Get194(thing));
}

u32 __cdecl Panel7cc258_ApplyToThing(u8 *thing) {
    const u8 *const shown = *reinterpret_cast<u8 *const *>((uintptr_t)0x007c2658);
    if (*reinterpret_cast<const u16 *>(thing + 0xe) == *reinterpret_cast<const u16 *>(shown + 0xe)) {
        Set198_4e21e0(thing, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc280));
        Object_SetQualityOfGoods(thing, nullptr, (u8)(*reinterpret_cast<const s8 *>((uintptr_t)0x007cc27c) * 0x32));   // (IMUL CL: the low byte)
        Obj_Set194(thing, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc284));
    }
    return 1;
}
}
