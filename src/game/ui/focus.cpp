#include "focus.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../core/vcall.h"
#include "../files/wrapsources.h"
#include "../world/mousemode.h"
#include "cursor.h"
#include "dialog7ca1c8.h"
#include "dialogbase.h"
#include "dialogs.h"
#include "window.h"
#include "windowfactory.h"
#include "dialog74fb20.h"
#include "window7cc8bc.h"
#include "panel7c2534menus.h"
#include "panel7b9940.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

u32 focus(u8 *window) { return Window_SetFocus(window, nullptr, 0); }

bool shown(u32 flags) { return (flags & 1) != 0; }

void enable(u32 map) { KeyMap_Enable(global<u8 *>(map)); }

}  // namespace

extern "C" {

u32 __cdecl Global7c2534_HasSource() {
    const TrackedSource *const source = reinterpret_cast<const TrackedSource *>(gPanel7c2534menus_7c2534);
    return source && Source_Field0c(source) ? 1 : 0;
}

u32 __cdecl Focus_Restore() {
    if (Global7c2534_HasSource()) return focus(*reinterpret_cast<u8 **>(gPanel7c2534menus_7c2534 + 0x10));
    if (!gPanel7b9940_7cb2ac) {
        if (gPanel7b9940_7cb2b0 && shown(Fn_65d57c(gPanel7b9940_7cb2b0))) {
            enable(0x00787c14);
            return focus(gPanel7b9940_7cb2b0);
        }
        if (gWindow7cc8bc_7cc8bc && shown(Fn_65d57c(gWindow7cc8bc_7cc8bc))) return focus(gWindow7cc8bc_7cc8bc);
        if (gWindow7cc8bc_7cc8c0 && shown(Fn_65d57c(gWindow7cc8bc_7cc8c0))) return focus(gWindow7cc8bc_7cc8c0);
        u8 *const frame = global<u8 *>(0x00f82884);
        if (!frame) return 0;
        if (!*reinterpret_cast<u8 **>(frame + 4)) return (u32)(uintptr_t)frame;
        return focus(*reinterpret_cast<u8 **>(frame + 4));
    }
    if (reinterpret_cast<u8 * &>(gDialog74fb20_7c24cc)) return focus(reinterpret_cast<u8 * &>(gDialog74fb20_7c24cc));
    if (shown(Fn_65d57c(gPanel7b9940_7cb2b0))) {
        enable(0x00787c14);
        return focus(gPanel7b9940_7cb2b0);
    }
    u32 flags = Fn_65d57c(gPanel7b9940_7cb2ac);
    if (shown(flags)) {
        if (gFocus_7c24c8) return focus(gFocus_7c24c8);
        if (Global7ca1cc_Get()) {
            enable(0x00787c14);
            return Dialog7ca1c8_Focus();
        }
        enable(0x00787c14);
        enable(0x00787a60);
        enable(0x0078733c);
        return focus(gPanel7b9940_7cb2ac);
    }
    if (!gFocus_7cb2b4) return flags;
    flags = Fn_65d57c(gFocus_7cb2b4);
    if (!shown(flags)) return flags;
    enable(0x00787c14);
    return focus(gFocus_7cb2b4);
}

u32 __cdecl Window7c24c8_Close() {
    u8 *const window = gFocus_7c24c8;
    if (!window) return 0;
    Window_SendMessage(window, 5, 0, 0);
    gFocus_7c24c8 = nullptr;
    Focus_Restore();
    return 1;
}

u32 __cdecl Window7c24c8_Set(u8 *window) {
    if (window) {
        MouseModes_EnsureHolder();
        u8 *const current = global<u8 *>(0x007b05d8);
        if ((current ? vcall<u32>(current, 0x24) : 0) == 9) {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
            u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
            MouseModes_EnsureHolder();
            MouseMode_Set(mode);
        }
        GameCursor_Set(0);
        KeyMap_Reset(global<u8 *>(0x00787c14));
        KeyMap_Reset(global<u8 *>(0x0078733c));
        KeyMap_Reset(global<u8 *>(0x00787a60));
        Dialog7cb2b8_ResetChild21(1);
    }
    gFocus_7c24c8 = window;
    return Focus_Restore();
}

u8 *__cdecl Window7cb2cc_Open() {
    if (!gFocus_7cb2cc) gFocus_7cb2cc = Dialog_Create(gWindow7cb2cc_Open_752e00, 0x004a2680, 1);
    return gFocus_7cb2cc;
}

u8 *__cdecl Window7cb2cc_OpenOnGui() {
    if (!global<u8 *>(0x00faa5d8)) return nullptr;
    Fn_662411();
    return Window7cb2cc_Open();
}

u32 __cdecl Window7cb2cc_Close() {
    if (u8 *const window = gFocus_7cb2cc) Window_SendMessage(window, 4, 0, 0);
    gFocus_7cb2cc = nullptr;
    return Focus_Restore();
}

u32 __cdecl Window7cb2cc_Toggle() {
    if (gFocus_7cb2cc) return Window7cb2cc_Close();
    return (u32)(uintptr_t)Window7cb2cc_Open();
}

u32 __cdecl Overlay7cb2b4_Toggle() {
    u32 &on = gFocus_7cb2e8;
    u8 *&overlay = gFocus_7cb2b4;
    if (!gPanel7b9940_7cb2ac) return 0;
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
    Window_SetShown(gPanel7b9940_7cb2ac, nullptr, (u16)on);
    const u32 old = on;
    on = 1 - old;
    return on ? old : Focus_Restore();
}

u32 __fastcall Obj7b51f0_ToggleOverlay(void *) { return Overlay7cb2b4_Toggle(); }

u32 __fastcall Obj7b51f0_CloseOverlay(void *) { return Global7cb2e8_Get() ? Overlay7cb2b4_Toggle() : 0; }
}
