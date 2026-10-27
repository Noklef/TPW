#include "panel7cc298.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../world/advisor.h"
#include "../world/object.h"
#include "dialogs.h"
#include "panelparts.h"
#include "window.h"
#include "window705a50.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using Slot = void(__fastcall *)(u8 *, void *, u32);

u8 *window14(u8 *self) { return at<u8 *>(self, 0x14); }

u8 *child(u8 *self, u32 id) { return Window_FindChild(at<u8 *>(self, 0x10), nullptr, id); }

}  // namespace

extern "C" {

void __fastcall Panel7cc298_Update(u8 *self) {
    const u8 *const thing = global<const u8 *>(0x007c2658);
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

void __fastcall Panel7cc298_Show(u8 *self, void *, u8 *thing) {
    *reinterpret_cast<u8 **>((uintptr_t)0x007cc2c8) = thing;
    WindowAt10_Create(self, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00755e80), 0x004b2150, 0xa0a2, 0xa091, 0xa0a3);
    Advisor_PostId(0xd0);
}

void __fastcall Panel7cc298_ApplyToShown(u8 *self) {
    u8 *const thing = global<u8 *>(0x007c2658);
    Obj_Set194(thing, nullptr, at<u32>(self, 0x2c));
    if (!at<u32>(Object_GetDef(thing), 0x10c)) {
        Object_SetChanceOfWinning(thing, nullptr, at<u8>(self, 0x24));
        Obj_Set188(thing, nullptr, at<u32>(self, 0x28));
    }
}

u32 __cdecl Panel7cc298_ApplyToThing(u8 *thing) {
    if (at<u16>(thing, 0xe) == at<u16>(global<const u8 *>(0x007c2658), 0xe)) {
        Obj_Set194(thing, nullptr, global<u32>(0x007cc2c4));
        if (!at<u32>(Object_GetDef(thing), 0x10c)) {
            Object_SetChanceOfWinning(thing, nullptr, global<u8>(0x007cc2bc));
            Obj_Set188(thing, nullptr, global<u32>(0x007cc2c0));
        }
    }
    return 1;
}
}
