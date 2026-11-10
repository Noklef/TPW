#include "panel7cc298.h"

#include <cstdint>
#include <cstring>

#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../core/language.h"
#include "../core/textarg.h"
#include "../core/vcall.h"
#include "../world/advisor.h"
#include "../world/analyser.h"
#include "../world/object.h"
#include "../world/objectstatus.h"
#include "../world/ridesweep.h"
#include "../world/world.h"
#include "dialogs.h"
#include "guirect.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"
#include "window705a50.h"
#include "window705af0.h"
#include "window705b20.h"
#include "panel7cc218.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using Slot = void(__fastcall *)(u8 *, void *, u32);

u8 *window14(u8 *self) { return at<u8 *>(self, 0x14); }

u8 *child(u8 *self, u32 id) { return Window_FindChild(at<u8 *>(self, 0x10), nullptr, id); }

}  // namespace

extern "C" {

void __fastcall Panel7cc298_Update(u8 *self) {
    const u8 *const thing = gPanel7cc218_7c2658;
    void *const *table = at<void *const *>(at<u8 *>(self, 0x20), 0);
    const u32 value = vcall<u32>(self, 0x34);
    // sic: the object's vtable from before the call, the object read again after it
    reinterpret_cast<Slot>(table[9])(at<u8 *>(self, 0x20), nullptr, value);
    vcall<void>(self, 0x18);
    vcall<void>(self, 0x14);
    if (u8 *const open = child(self, 0xa08a))
        Window705a50_SetState(open, nullptr, Is68_Zero_4e0440(thing) ? 1 : 0, 1);
    u8 *c = child(self, 0xa08e);
    table = at<void *const *>(c, 0);
    reinterpret_cast<Slot>(table[7])(c, nullptr, GetByte190_4e21b0(thing));
    c = child(self, 0xa08f);
    table = at<void *const *>(c, 0);
    reinterpret_cast<Slot>(table[7])(c, nullptr, Obj_Get194(thing));
    c = child(self, 0xa090);
    table = at<void *const *>(c, 0);
    reinterpret_cast<Slot>(table[7])(c, nullptr, Obj_Get188(thing));
    const u32 hidden = at<u32>(Object_GetDef(thing), 0x10c) != 0;
    static const u32 kShown[] = {0xa090, 0xa08e, 0xa0a6, 0xa0a7};
    for (const u32 id : kShown)
        Window_SetShown(child(self, id), nullptr, hidden ? 0 : 1);
    c = child(self, 0xa0a5);
    table = at<void *const *>(c, 0);
    reinterpret_cast<Slot>(table[9])(c, nullptr, (u32)(uintptr_t)Language_WideText(hidden ? 0x34 : 0x33));
}

void __fastcall Panel7cc298_Dress(u8 *self) {
    using namespace panelparts;
    if (!window14(self)) return;
    text(window14(self), 0xa09c, 0x29);
    text(window14(self), 0xa095, 0x2a);
    text(window14(self), 0xa09a, 0x2b);
    text(window14(self), 0xa098, 0x2c);
    text(window14(self), 0xa09b, 0x2d);
    text(window14(self), 0xa097, 0x2e);
    text(window14(self), 0xa0a1, 0x2f);
    text(window14(self), 0xa09f, 0x30);
    textNone(window14(self), 0xa09d);
    bar(window14(self), kRideStatBar, 0xa096, 0x32);
    part(window14(self), TextPart6ff3b0_Ctor, 0xa093, 0);
    part(window14(self), TextPart6ff3b0_Ctor, 0xa092, 0);
    bar(window14(self), kRideStatBar, 0xa099, 0x64);
    bar(window14(self), kRideStatBar, 0xa094, 0x32);
    part(window14(self), TextPart6ff3b0_Ctor, 0xa0a0, 0);
    bar(window14(self), kHappyGrad, 0xa09e, 0x64);
}

void __fastcall Panel7cc298_DressTip(u8 *self) { panelparts::tip(self); }

void __fastcall Panel7cc298_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, *reinterpret_cast<const u32 *>((uintptr_t)0x007cc2c8)) < 2) {
        Window_SetShown(child(self, 0xa08c), nullptr, 0);
        Window_SetShown(child(self, 0xa08d), nullptr, 0);
    }
    u8 rect[12];
    GuiRect_Ctor(rect);
    if (u8 *const slider = child(self, 0xa08e)) Window705b20_SetRange(slider, nullptr, 0, 100);
    const auto value = [self](u32 id, s32 high) {
        u8 *const value = child(self, id);
        if (!value) return;
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(value, nullptr, memory ? TextPart6ff3b0_Ctor(memory, nullptr, 0) : nullptr);
        const s32 low = 0, step = 1;
        Window705af0_SetRange(value, nullptr, &low, &high);
        Window705af0_SetStep(value, nullptr, &step);
        Window_Set4c(value, nullptr, Language_Font(6));
        Window_NotifyC00(value, nullptr, 1, 1);
        Window_Notify40000(value, nullptr, 0, 0, 0, 0xff);
    };
    const auto label = [self](u32 id, u32 text) {
        u8 *const label = child(self, id);
        if (!label) return;
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x1a4));
        Window_SetImpl(label, nullptr, memory ? TextPart_Ctor(memory, nullptr, 0) : nullptr);
        Window_NotifyC00(label, nullptr, 1, 1);
        Window_Set4c(label, nullptr, Language_Font(6));
        Window_Notify40000(label, nullptr, 0, 0, 0, 0xff);
        Window_NotifyImpl100(label, nullptr, (u32)(uintptr_t)Language_WideText(text));
    };
    value(0xa090, 10000);
    value(0xa08f, 500);
    label(0xa0a7, 0x31);
    label(0xa0a6, 0x32);
    label(0xa0a5, 0x33);
    HoverLabel_Bind(self, nullptr, 0xa0a4);
    GuiRect_Dtor(rect);
}

void __fastcall Panel7cc298_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc2c8) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)&gPanel7cc298_Show_755e80), 0x004b2150, 0xa0a2, 0xa091, 0xa0a3);
    Advisor_PostId(0xd0);
}

void __fastcall Panel7cc298_ApplyToShown(u8 *self) {
    u8 *const thing = gPanel7cc218_7c2658;
    Obj_Set194(thing, nullptr, at<u32>(self, 0x2c));
    if (!at<u32>(Object_GetDef(thing), 0x10c)) {
        Object_SetChanceOfWinning(thing, nullptr, at<u8>(self, 0x24));
        Obj_Set188(thing, nullptr, at<u32>(self, 0x28));
    }
}

u32 __cdecl Panel7cc298_ApplyToThing(u8 *thing) {
    if (at<u16>(thing, 0xe) == at<u16>(gPanel7cc218_7c2658, 0xe)) {
        Obj_Set194(thing, nullptr, global<u32>(0x007cc2c4));
        if (!at<u32>(Object_GetDef(thing), 0x10c)) {
            Object_SetChanceOfWinning(thing, nullptr, global<u8>(0x007cc2bc));
            Obj_Set188(thing, nullptr, global<u32>(0x007cc2c0));
        }
    }
    return 1;
}

void __fastcall Panel7cc298_ShowStats(u8 *self) {
    u8 *const thing = gPanel7cc218_7c2658;
    Object_GetDef(thing);  // (its result not used)
    if (window14(self)) {
        // (each sum's debug print of whether 30 is within the size is left out)
        u8 *const text = Window_FindChild(window14(self), nullptr, 0xa09d);
        u32 seen0, seen1;
        const double sum0 = CircBufU32_30_SumVia(thing + 0x1a8, &seen0);
        const double sum1 = CircBufU32_30_SumVia(thing + 0x230, &seen1);
        u8 both[0x14], recent[0x14];   // (number arguments, as TextArgInt_Ctor builds them inline)
        at<u32>(both, 0) = (u32)(uintptr_t)TextArgInt_vtable;
        at<u32>(both, 4) = 0x5641524d;
        at<u32>(both, 8) = 0x1a;
        at<u32>(both, 0xc) = (u32)(long long)sum0 + (u32)(long long)sum1;  // (__ftol, the low halves)
        at<u32>(both, 0x10) = 0;
        // the slot the later look backs write to: until then the low half of the first sum (it was stored there)
        u32 slot;
        std::memcpy(&slot, &sum0, 4);
        const double sum2 = CircBufU32_30_SumVia(thing + 0x1a8, &slot);
        at<u32>(recent, 0) = (u32)(uintptr_t)TextArgInt_vtable;
        at<u32>(recent, 4) = 0x5641524d;
        at<u32>(recent, 8) = 0x19;
        at<u32>(recent, 0xc) = (u32)(long long)sum2;
        at<u32>(recent, 0x10) = 0;
        TextFormat().format(global<void *>(0x0078ba94), reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff, 0x1cc, recent, both);
        vcall<void>(text, 0x24, 0x007ba058u);  // (not checked)
        if (u8 *const bar = Window_FindChild(window14(self), nullptr, 0xa096))
            vcall<void>(bar, 0x1c, (u32)((s32)(Object_HistoryPercent(thing) << 10) / 100));
        if (u8 *const part = Window_FindChild(window14(self), nullptr, 0xa093)) {
            const double sum3 = CircBufU32_30_SumVia(thing + 0x70, &slot);
            vcall<void>(part, 0x1c, (u32)(long long)sum3);
        }
        if (u8 *const part = Window_FindChild(window14(self), nullptr, 0xa092))
            vcall<void>(part, 0x1c, Object_HistoryDifference(thing));
        if (u8 *const bar = Window_FindChild(window14(self), nullptr, 0xa099)) {
            const u32 rating = Object_ComputeRating(thing, nullptr, at<s32>(self, 0x28), at<s32>(self, 0x2c), at<s32>(self, 0x24));
            vcall<void>(bar, 0x1c, (u32)((s32)(rating << 10) / 100));
        }
        if (u8 *const bar = Window_FindChild(window14(self), nullptr, 0xa094))
            vcall<void>(bar, 0x1c, (u32)((s32)(Object_HistoryAverage(thing) << 10) / 100));
        u8 *const refund = Window_FindChild(window14(self), nullptr, 0xa0a0);  // (not checked)
        vcall<void>(refund, 0x1c, Object_ScrapRefund(thing));
        u8 *const grad = Window_FindChild(window14(self), nullptr, 0xa09e);  // (not checked)
        // the cell id a word over the slot's upper half (sic: the dword is passed)
        const u32 cell = (slot & 0xffff0000u) | (u16)(((u32)thing[7] << 7) + thing[5] + 1);
        u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(gWorldPtr));
        const s32 average = ParkAnalyser_AveragePersonStat19c(analyser, nullptr, cell, 1);
        vcall<void>(grad, 0x1c, (u32)((s32)((u32)average << 10) / 100));
    }
    const u32 status = Object_Status(thing, 2);
    if (status != at<u32>(self, 8)) {
        at<u32>(self, 8) = status;
        vcall<void>(self, 0x24);
    }
}
}
