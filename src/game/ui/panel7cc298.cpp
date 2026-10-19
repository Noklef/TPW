#include "panel7cc298.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../core/vcall.h"
#include "../world/object.h"
#include "window.h"
#include "window705a50.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

using Slot = void(__fastcall *)(u8 *, void *, u32);

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
}
