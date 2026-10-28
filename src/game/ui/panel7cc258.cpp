#include "panel7cc258.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../world/advisor.h"
#include "../world/analyser.h"
#include "../world/object.h"
#include "../world/objectstatus.h"
#include "../world/ridesweep.h"
#include "../world/things.h"
#include "../world/world.h"
#include "dialogs.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window705a50.h"
#include "window705af0.h"
#include "window705b20.h"

using namespace panelparts;

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
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

void __fastcall Panel7cc258_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc288) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00755908), 0x004b0b30, 0xc06c, 0xc06f, 0xc06d);
    Advisor_PostId(0xd1);
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

void __fastcall Panel7cc258_ShowStats(u8 *self) {
    u8 *const thing = global<u8 *>(0x007c2658);
    Object_GetDef(thing);  // (its result not used)
    if (at<u8 *>(self, 0x14)) {
        // (each sum's debug print of whether 30 is within the size is left out; no child is checked)
        u8 *const text = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc070);
        u32 seen0, seen1, seen2;
        const double sum0 = CircBufU32_30_SumVia(thing + 0x1a8, &seen0);
        const double sum1 = CircBufU32_30_SumVia(thing + 0x230, &seen1);
        u8 both[0x14], recent[0x14];   // (number arguments, as TextArgInt_Ctor builds them inline)
        at<u32>(both, 0) = 0x006ff170;
        at<u32>(both, 4) = 0x5641524d;
        at<u32>(both, 8) = 0x1a;
        at<u32>(both, 0xc) = (u32)(long long)sum0 + (u32)(long long)sum1;  // (__ftol, the low halves)
        at<u32>(both, 0x10) = 0;
        const double sum2 = CircBufU32_30_SumVia(thing + 0x1a8, &seen2);
        at<u32>(recent, 0) = 0x006ff170;
        at<u32>(recent, 4) = 0x5641524d;
        at<u32>(recent, 8) = 0x19;
        at<u32>(recent, 0xc) = (u32)(long long)sum2;
        at<u32>(recent, 0x10) = 0;
        TextFormat().format(global<void *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, 0x1cc, recent, both);
        vcall<void>(text, 0x24, 0x007ba058u);
        u8 *const settings = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc076);
        // the stack slot the ring +0x70's look backs write to next: first the setting +0x24 * 50
        u32 slot = at<u32>(self, 0x24) * 50;
        const u32 value = Object_SettingsValue(thing, nullptr, (float)(s32)slot, (float)at<s32>(self, 0x28));
        vcall<void>(settings, 0x1c, value);
        u8 *const part = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc077);
        const double sum3 = CircBufU32_30_SumVia(thing + 0x70, &slot);
        vcall<void>(part, 0x1c, (u32)(long long)sum3);
        u8 *const average = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc079);
        vcall<void>(average, 0x1c, (u32)((s32)(Object_HistoryAverage(thing) << 10) / 100));
        u8 *const difference = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc078);
        vcall<void>(difference, 0x1c, Object_HistoryDifference(thing));
        u8 *const refund = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc07d);
        vcall<void>(refund, 0x1c, Object_ScrapRefund(thing));
        u8 *const grad = Window_FindChild(at<u8 *>(self, 0x14), nullptr, 0xc07c);
        // the cell id a word over the slot's upper half (sic: the dword is passed)
        const u32 cell = (slot & 0xffff0000u) | (u16)(((u32)thing[7] << 7) + thing[5] + 1);
        u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x007cf83c)));
        const s32 people = ParkAnalyser_AveragePersonStat19c(analyser, nullptr, cell, 1);
        vcall<void>(grad, 0x1c, (u32)((s32)((u32)people << 10) / 100));
    }
    const u32 status = Object_Status(thing, 2);
    if (status != at<u32>(self, 8)) {
        at<u32>(self, 8) = status;
        vcall<void>(self, 0x24);
    }
}
}
