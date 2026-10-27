// The info panel a thing is shown in: see thingpanel.h.
#include "thingpanel.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../world/object.h"
#include "panel7ca178.h"
#include "panel7ca228.h"
#include "panel7cc218.h"
#include "panel7cc258.h"
#include "panel7cc298.h"
#include "panel7cc328.h"
#include "panel7cc3b0.h"
#include "panel7cc3e0.h"
#include "panel7cc408.h"

namespace {

template <typename T> T &at(const void *base, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)base + offset); }
u8 *Panel(u32 address) { return reinterpret_cast<u8 *>((uintptr_t)address); }

}  // namespace

void __cdecl ThingPanel_Show(u8 *thing) {
    if (thing == nullptr) return;
    const u8 kind = at<u8>(thing, 2);
    if (kind == 3) {
        u8 *const def = Object_GetDef(thing);
        switch (at<u32>(def, 0x4ac)) {
        case 0: {
            if (at<u32>(def, 0x288) == 2) {
                Panel7cc3e0_Show(Panel(0x007cc3e0), nullptr, thing);
                return;
            }
            u16 link = 0;
            Fn_4e0df0(thing, nullptr, (u32)(uintptr_t)&link);
            if (link != 0) {
                Fn_4e0df0(thing, nullptr, (u32)(uintptr_t)&link);
                // (Debug_Printf when the thing table 0x7cfb90 is empty, left out)
                Panel7cc218_Show(Panel(0x007cc218), nullptr, at<u8 *>(Panel(0x007cfb90), (u32)link * 0x14));
                return;
            }
            u16 word;
            if (*Object_DefWord284(thing, nullptr, &word) == 0) Panel7cc218_Show(Panel(0x007cc218), nullptr, thing);
            return;
        }
        case 1: Panel7cc258_Show(Panel(0x007cc258), nullptr, thing); return;
        case 2: Panel7cc298_Show(Panel(0x007cc298), nullptr, thing); return;
        case 3: {
            const u16 flags = at<u16>(thing, 0x32);
            if (flags & 1)
                Panel7ca178_Show(Panel(0x007ca178), nullptr, thing);
            else if (flags & 2)
                Panel7cc328_Show(Panel(0x007cc328), nullptr, thing);
            else
                Panel7ca228_Show(Panel(0x007ca228), nullptr, thing);
            return;
        }
        default: return;
        }
    }
    if (kind == 5 || kind == 4 || kind == 6 || kind == 7 || kind == 8)
        Panel7cc3b0_Show(Panel(0x007cc3b0), nullptr, thing);
    else if (kind == 1)
        Panel7cc408_Show(Panel(0x007cc408), nullptr, thing);
}
