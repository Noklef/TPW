#include "focus.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/keymap.h"
#include "../files/wrapsources.h"
#include "dialog7ca1c8.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

u32 focus(u8 *window) { return Window_SetFocus(window, nullptr, 0); }

bool shown(u32 flags) { return (flags & 1) != 0; }

void enable(u32 map) { KeyMap_Enable(global<u8 *>(map)); }

}  // namespace

extern "C" {

u32 __cdecl Global7c2534_HasSource() {
    const TrackedSource *const source = global<const TrackedSource *>(0x007c2534);
    return source && Source_Field0c(source) ? 1 : 0;
}

u32 __cdecl Focus_Restore() {
    if (Global7c2534_HasSource()) return focus(*reinterpret_cast<u8 **>(global<u8 *>(0x007c2534) + 0x10));
    if (!global<u8 *>(0x007cb2ac)) {
        if (global<u8 *>(0x007cb2b0) && shown(Fn_65d57c(global<u8 *>(0x007cb2b0)))) {
            enable(0x00787c14);
            return focus(global<u8 *>(0x007cb2b0));
        }
        if (global<u8 *>(0x007cc8bc) && shown(Fn_65d57c(global<u8 *>(0x007cc8bc)))) return focus(global<u8 *>(0x007cc8bc));
        if (global<u8 *>(0x007cc8c0) && shown(Fn_65d57c(global<u8 *>(0x007cc8c0)))) return focus(global<u8 *>(0x007cc8c0));
        u8 *const frame = global<u8 *>(0x00f82884);
        if (!frame) return 0;
        if (!*reinterpret_cast<u8 **>(frame + 4)) return (u32)(uintptr_t)frame;
        return focus(*reinterpret_cast<u8 **>(frame + 4));
    }
    if (global<u8 *>(0x007c24cc)) return focus(global<u8 *>(0x007c24cc));
    if (shown(Fn_65d57c(global<u8 *>(0x007cb2b0)))) {
        enable(0x00787c14);
        return focus(global<u8 *>(0x007cb2b0));
    }
    u32 flags = Fn_65d57c(global<u8 *>(0x007cb2ac));
    if (shown(flags)) {
        if (global<u8 *>(0x007c24c8)) return focus(global<u8 *>(0x007c24c8));
        if (Global7ca1cc_Get()) {
            enable(0x00787c14);
            return Dialog7ca1c8_Focus();
        }
        enable(0x00787c14);
        enable(0x00787a60);
        enable(0x0078733c);
        return focus(global<u8 *>(0x007cb2ac));
    }
    if (!global<u8 *>(0x007cb2b4)) return flags;
    flags = Fn_65d57c(global<u8 *>(0x007cb2b4));
    if (!shown(flags)) return flags;
    enable(0x00787c14);
    return focus(global<u8 *>(0x007cb2b4));
}
}
