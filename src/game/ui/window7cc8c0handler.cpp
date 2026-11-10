// The handler of the online view's windows: see window7cc8c0handler.h.
#include "window7cc8c0handler.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../core/vcall.h"
#include "../core/vwrap.h"
#include "../files/charsets.h"
#include "../frontend/helpbg.h"
#include "../input/mouse.h"
#include "../model/highlight.h"
#include "../online/chat.h"
#include "../online/chatavatar.h"
#include "../world/camera.h"
#include "../world/mousemode.h"
#include "../world/object.h"
#include "../world/thingscan.h"
#include "cursor.h"
#include "dialog7ca210.h"
#include "guirect.h"
#include "window.h"
#include "dialogs.h"
#include "window7cc8bc.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &field(u8 *object, u32 offset) { return *reinterpret_cast<T *>(object + offset); }

u8 *&view() { return gWindow7cc8bc_7cc8bc; }
u8 *&viewB() { return gWindow7cc8bc_7cc8c0; }
u32 &hover() { return gWindow7cc8bc_7cc8e4; }
u8 *table() { return gKeymap_787c14; }

const Window7cc8c0HandlerEnv kGameEnv = {
    Window_GetRect,
    HelpBg_Place,
    Window_SendMessage,
    Window_PostMessage,
    Window_CaptureMouse,
    Window_ReleaseMouse,
    reinterpret_cast<void(__cdecl *)(u32, u32, u32)>(&Camera_Input),
    OnlineChat_Ctor,
    ChatAvatar_SayPosition,
    OnlineChat_Instance,
    ChatAvatar_MoveTo,
    ChatAvatar_MoveTo3,
    Map_ObjectOrEntranceAt,
    Object_DefWord284,
    Object_GetDef,
    Object_FollowCamera,
    HelpBg_SetText,
    Charset_FormatWide,
    Thing_NameBuffer7ba058,
    Highlight_Update,
    Highlight_Clear,
    KeyMap_Modifiers,
    Actions_FireDown,
    Actions_FireUp,
    Window_SetFocus,
    Window_KillFocus,
    reinterpret_cast<s32(__fastcall *)(const u8 *, void *, u32, u32)>(&KeyMap_FindKey),
    reinterpret_cast<void(__cdecl *)(u32, u32, s32, s32, u32)>(&Mouse_HandleButton),
    reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x004a9380),
    reinterpret_cast<void(__cdecl *)(u32)>((uintptr_t)0x0048c830),
    Window_CallDefault,
};
const Window7cc8c0HandlerEnv *g_env = &kGameEnv;

u32 ModeKind() {
    u8 *const mode = global<u8 *>(0x007b05d8);
    return mode ? vcall<u32>(mode, 0x24) : 0;
}

// The edit box under a window: its child 0xc3123's child 0xc312b's child 0xc312c.
u8 *EditBox(u8 *window) { return Window_FindChild(Window_FindChild(Window_FindChild(window, nullptr, 0xc3123), nullptr, 0xc312b), nullptr, 0xc312c); }

void FollowPosition() {
    u32 flags = gCameraFlags;
    const u32 last = gWindow7cc8bc_7cc8ac;
    const bool on = (flags & 0x400) != 0;
    bool left = false;
    if (on && !(last & 0x400)) {
        gWindow7cc8bc_7cc8d4 = 0;
        u8 *avatar = gChat_7cddec;
        if (!avatar) {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0xf0c));
            avatar = block ? g_env->chatCtor(block) : nullptr;
            gChat_7cddec = avatar;
        }
        g_env->say(avatar);
        flags = gCameraFlags;
    } else if (!on && (flags & 0x16) && (last & 0x400)) {
        gWindow7cc8bc_7cc8d4 = 1;
        left = true;
        gWindow7cc8c0handler_7cc8d8 = 0;
    }
    const u32 timer = gWindow7cc8c0handler_7cc8d8;
    gWindow7cc8bc_7cc8ac = flags;
    if (timer) {
        gWindow7cc8c0handler_7cc8d8 = timer - 1;
        if (timer - 1) return;
    }
    if (!gWindow7cc8bc_7cc8d4) return;
    u16 x, z;
    if (left) {
        Camera_PackCellFractions(global<u32>(0x007908f0), global<u32>(0x007908f8), reinterpret_cast<u8 *>(&x), reinterpret_cast<u8 *>(&z));
    } else {
        Camera_PackCellFractions(reinterpret_cast<u32 &>(gCamera_790aa0[0]), reinterpret_cast<u32 &>(gCamera_790aa0[2]), reinterpret_cast<u8 *>(&x), reinterpret_cast<u8 *>(&z));
    }
    if (x == global<u16>(0x007cc690) && z == global<u16>(0x007cc694) && !left) return;
    global<u16>(0x007cc690) = x;
    global<u16>(0x007cc694) = z;
    // (the original pushes both as dwords, their high words uninitialised stack; the callees read the low words)
    if (left) {
        g_env->moveTo3(g_env->chat(), nullptr, x, z);
    } else {
        g_env->moveTo(g_env->chat(), nullptr, x, z);
    }
}

// The object under the mouse, when it counts as one to follow.
bool OverObject() {
    const u32 tile = global<u16>(0x007b05cc);
    if (!tile) return false;
    u32 found;
    g_env->objectAt(reinterpret_cast<u16 *>(&found), (tile - 1) & 0x7f, (tile - 1) >> 7);
    if (!static_cast<u16>(found)) return false;
    const u32 handle = static_cast<u16>(found);
    u8 *const object = *reinterpret_cast<u8 **>((uintptr_t)(0x007cfb90 + handle * 20));
    if (object[2] != 3) return false;
    u16 word;
    if (*g_env->defWord(object, nullptr, &word) != 0) return false;
    bool over = field<u32>(g_env->def(object), 0x4ac) == 0;
    if (!over) over = field<u32>(g_env->def(object), 0x4ac) == 3 && (object[0x32] & 0x10);
    if (!over) return false;
    hover() = 2;
    gWindow7cc8c0handler_7cc8e8 = object;
    return true;
}

void ShowHelp(u8 *window) {
    if (!(Fn_65d57c(window) & 1)) return;
    const u32 before = hover();
    bool over = false;
    hover() = 0;
    if (gWindow7cc8c0handler_7cc8dc || gWindow7cc8c0handler_7cc8e0) {
        over = OverObject();
        MouseModes_EnsureHolder();
        const u32 kind = ModeKind();
        if (kind == 9) {
            hover() = hover() == 2 ? 4 : 3;
        } else if (hover() == 0) {
            hover() = 1;
        }
    }
    const u32 state = hover();
    if (state != before && state <= 4) {
        u16 *const text = &global<u16>(0x007ba058);
        u8 *const charset = global<u8 *>(0x0078ba98);
        u32 cursor = 0;
        switch (state) {
        case 0:
            g_env->setText(&reinterpret_cast<u16 &>(gWindow7cc8c0handler_78a374));
            break;
        case 1:
            g_env->format(charset, text, 0x3fff, 0x191);
            g_env->setText(text);
            break;
        case 3:
            g_env->format(charset, text, 0x3fff, 0x243);
            g_env->setText(text);
            cursor = 0x13;
            break;
        default: {   // 2, 4
            const u32 name = (u32)(uintptr_t)g_env->name(gWindow7cc8c0handler_7cc8e8);
            const u32 arg[4] = {0x006ff174, 0x5641524d, 2, name};
            g_env->format(charset, text, 0x3fff, 0x244, arg);
            g_env->setText(text);
            cursor = 0x13;
            break;
        }
        }
        Window_SetCursor(window, nullptr, cursor);
    }
    if (over) {
        g_env->highlight();
    } else {
        g_env->unhighlight();
    }
}

s32 ScaledX(u32 b) { return static_cast<s32>(b & 0xffff) * global<s16>(0x00faa5c0) / 1536; }
s32 ScaledY(u32 b) { return (static_cast<s32>(b) >> 16) * global<s16>(0x00faa5c4) / 2048; }

void MouseDown(u8 *window, u32 a, u32 b) {
    const u32 modifiers = g_env->modifiers();
    g_env->fireDown(table(), nullptr, a - 0x10, modifiers);
    bool button = true;
    if (a == 0) {
        MouseModes_EnsureHolder();
        const u32 kind = ModeKind();
        if (kind != 9 && hover() == 2) {
            const u8 *const object = gWindow7cc8c0handler_7cc8e8;
            if (object && object[2] == 3) {
                g_env->follow(object, nullptr, 0);
                button = false;
            }
        } else if (view() == window && static_cast<u16>(g_env->setFocus(window, nullptr, 0))) {
            button = false;
        }
    }
    if (!button) return;
    MouseModes_EnsureHolder();
    g_env->button(0xf0 + (a != 0), 1, ScaledY(b), ScaledX(b), 0);
}

// 0x73: the view swapped.
void SwapView() {
    const u8 shown = gWindow7cc8c0handler_7cc8b4;
    g_env->send(view(), 6, shown == 0, 0);
    g_env->send(viewB(), 6, shown, 0);
    if (gWindow7cc8c0handler_7cc8b4) {
        g_env->setFocus(EditBox(view()), nullptr, 0);
        MouseModes_EnsureHolder();
        if (ModeKind() == 9) Window_SetCursor(view(), nullptr, 0x13);
    } else {
        g_env->setFocus(viewB(), nullptr, 0);
    }
}

u8 *NewMode(u8 *(__fastcall *ctor)(u8 *)) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
    return block ? ctor(block) : nullptr;
}

void KeyDown(u8 *window, u32 a, u32 b) {
    const u32 high = static_cast<u32>(static_cast<s32>(b) >> 16);
    const u32 modifiers = high & 0xff;
    if (a == 9 || a == 13 || a == 0x2f) {
        if (!gWindow7cc8c0handler_7cc8b4 || modifiers != 0) {
            g_env->fireUp(table(), nullptr, a, modifiers);
            return;
        }
        g_env->killFocus(window, nullptr, 0);
        g_env->setFocus(EditBox(window), nullptr, 0);
        if (a == 0x2f) g_env->post(EditBox(window), 0x1000c, 0x2f, 0);
        return;
    }
    if (a == 0x73) {
        if (modifiers == 0 && !gDialogs_7cc6a0) SwapView();
        return;
    }
    const s32 action = static_cast<s16>(g_env->findKey(gKeymap_78721c, nullptr, a, modifiers));
    if (action == 0) {
        MouseModes_EnsureHolder();
        if (ModeKind() == 9) {
            u8 *const mode = NewMode(MMOnline_Ctor);
            MouseModes_EnsureHolder();
            MouseMode_Set(mode);
            Window_SetCursor(viewB(), nullptr, 0);
        } else {
            g_env->leave(0);
        }
    } else if (action == 0xf) {
        g_env->escape();
    } else if (action == 0x10) {
        u8 *const mode = NewMode(MouseMode6fead0_Ctor);
        MouseModes_EnsureHolder();
        MouseMode_Set(mode);
        Window_SetCursor(gWindow7cc8c0handler_7cc8b4 ? view() : viewB(), nullptr, 0x13);
    } else {
        g_env->fireUp(table(), nullptr, a, modifiers);
    }
}

}  // namespace

void Window7cc8c0Handler_SetEnv(const Window7cc8c0HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Window7cc8c0_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x11) {
        if (a == 1) {
            u8 rect[12];
            GuiRect_Ctor(rect);
            g_env->getRect(window, nullptr, rect, a);
            if (window == view()) gWindow7cc8c0handler_7cc8b4 = 1;
            GuiRect_Dtor(rect);
        } else if (window == view()) {
            gWindow7cc8c0handler_7cc8b4 = 0;
        }
    } else if (message == 0x14) {
        if (view() == window) {
            view() = nullptr;
            g_env->place(0xffffffffu, 0xffffffffu);
            g_env->send(viewB(), 4, 0, 0);
            g_env->send(gWindow7cc8bc_7cc8c8, 4, 0, 0);
            g_env->send(gWindow7cc8bc_7cc8c4, 4, 0, 0);
        }
        if (viewB() == window) viewB() = nullptr;
    } else if (message == 0x10004) {
        if (a == 1) g_env->release(window);
    } else if (message == 0x10005) {
        if (a == 1) g_env->capture(window);
    }
    g_env->cameraInput(message, a, b);
    switch (message) {
    case 0x1e:
        if (window == viewB()) FollowPosition();
        ShowHelp(window);
        break;
    case 0x10001:
        hover() = 0;
        global<u8>(window == viewB() ? reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_7cc8e0) : reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_7cc8dc)) = 1;
        break;
    case 0x10002:
        global<u8>(window == viewB() ? reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_7cc8e0) : reinterpret_cast<uintptr_t>(&gWindow7cc8c0handler_7cc8dc)) = 0;
        break;
    case 0x10004: {
        const u32 modifiers = g_env->modifiers();
        g_env->fireUp(table(), nullptr, a - 0x10, modifiers);
        break;
    }
    case 0x10005:
        MouseDown(window, a, b);
        break;
    case 0x10007:
        if (a == 0) {
            MouseModes_EnsureHolder();
            Global7b05d8_CallSlot14(ScaledY(b), ScaledX(b));
        }
        break;
    case 0x1000a:
        g_env->fireDown(table(), nullptr, a, (b >> 16) & 0xff);
        break;
    case 0x1000b:
        KeyDown(window, a, b);
        break;
    default:
        break;
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
}
