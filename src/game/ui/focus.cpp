#include "focus.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/keymap.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "dialog7ca1c8.h"
#include "window.h"
#include "windowfactory.h"

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

u32 __cdecl Window7c24c8_Close() {
    u8 *const window = global<u8 *>(0x007c24c8);
    if (!window) return 0;
    Window_SendMessage(window, 5, 0, 0);
    global<u8 *>(0x007c24c8) = nullptr;
    Focus_Restore();
    return 1;
}

u32 __cdecl Window7cb2cc_Close() {
    if (u8 *const window = global<u8 *>(0x007cb2cc)) Window_SendMessage(window, 4, 0, 0);
    global<u8 *>(0x007cb2cc) = nullptr;
    return Focus_Restore();
}

u32 __cdecl Overlay7cb2b4_Toggle() {
    u32 &on = global<u32>(0x007cb2e8);
    u8 *&overlay = global<u8 *>(0x007cb2b4);
    if (!global<u8 *>(0x007cb2ac)) return 0;
    if (on) {
        if (overlay) vcall<void>(overlay, 8, 1u);
        overlay = nullptr;
    } else {
        if (global<u8>(0x00790ab0) & 0x16) return 0;
        if (!overlay) {
            overlay = Window_CreateChild(global<u8 *>(0x00faa5d8), nullptr, 1, 1, 0xddf22, 0, 0, 0x7ff, 0x5ff);
            if (overlay) {
                Window_SetHandler(overlay, nullptr, 0x004a2840);
                focus(overlay);
            }
        }
    }
    Window_SetShown(global<u8 *>(0x007cb2ac), nullptr, (u16)on);
    const u32 old = on;
    on = 1 - old;
    return on ? old : Focus_Restore();
}

u32 __fastcall Obj7b51f0_ToggleOverlay(void *) { return Overlay7cb2b4_Toggle(); }

u32 __fastcall Obj7b51f0_CloseOverlay(void *) { return Global7cb2e8_Get() ? Overlay7cb2b4_Toggle() : 0; }
}
