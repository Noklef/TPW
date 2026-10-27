#include "panel7ca228.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../world/ridesweep.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Panel7ca228_Update(u8 *self) {
    const u8 *const thing = global<const u8 *>(0x007c2658);
    void *const *const table = at<void *const *>(at<u8 *>(self, 0x20), 0);
    const u32 value = vcall<u32>(self, 0x34);
    // sic: the object's vtable from before the call, the object read again after it
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(table[9])(at<u8 *>(self, 0x20), nullptr, value);
    const u32 shown = (at<u8>(thing, 0x32) & 0x10) ? 1 : 0;
    Window_SetShown(Window_FindChild(at<u8 *>(self, 0x10), nullptr, 0x1581), nullptr, shown);
    vcall<void>(self, 0x18);
    vcall<void>(self, 0x14);
}

void __fastcall Panel7ca228_Open(u8 *self) {
    if (Things_CountGroup(self, nullptr, global<u32>(0x007ca218)) < 2) {
        Window_SetShown(Window_FindChild(at<u8 *>(self, 0x10), nullptr, 0x157c), nullptr, 0);
        Window_SetShown(Window_FindChild(at<u8 *>(self, 0x10), nullptr, 0x157d), nullptr, 0);
    }
    HoverLabel_Bind(self, nullptr, 0x157f);
}

void __fastcall Panel7ca228_Dress(u8 *self) {
    using namespace panelparts;
    if (!at<u8 *>(self, 0x14)) return;
    textIfAny(at<u8 *>(self, 0x14), 0x1587, 0x35);
    partIfAny(at<u8 *>(self, 0x14), TextPart6ff350_Ctor, 0x1586, 0);
    textIfAny(at<u8 *>(self, 0x14), 0x1585, 0x36);
    partIfAny(at<u8 *>(self, 0x14), TextPart6ff3b0_Ctor, 0x1584, 0);
}

}
