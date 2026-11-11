// The handler of the window [0x7cb2ac]: see window7cb2achandler.h.
#include "window7cb2achandler.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/cheatmodule.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/keymap.h"
#include "../input/mouse.h"
#include "../model/highlight.h"
#include "../world/camera.h"
#include "../world/gamemode.h"
#include "../world/mousemode.h"
#include "dialog7ca1c8.h"
#include "focus.h"
#include "guimanager.h"
#include "guisettings.h"
#include "mouseclick.h"
#include "mousehelp.h"
#include "panel7b9940.h"
#include "window.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Window7cb2acHandlerEnv kGameEnv = {Window_CaptureMouse, Window_ReleaseMouse, GuiClock_Now, reinterpret_cast<void(__cdecl *)(u32, u32, u32)>(&Camera_Input), GameMode_Set,
                                         Dialog7ca1c8_Hide, Mouse_Moved, Highlight_Clear, MouseHelp_Update, KeyMap_Modifiers, Actions_FireDown, Actions_FireUp,
                                         reinterpret_cast<void(__cdecl *)(u32, u32, s32, s32, u32)>(&Mouse_HandleButton), MouseClick_OnShown, CheatSystem_Construct, CRT_atexit,
                                         GetBytea_405560, Window_CallDefault};
const Window7cb2acHandlerEnv *g_env = &kGameEnv;

u32 &toCamera() { return gWindow7cb2achandler_74fc2c; }
u32 &drag() { return global<u32>(0x007c2500); }
u32 &over() { return global<u32>(0x007c24fc); }

s32 ScaledX(u32 b) { return (static_cast<s32>(b) >> 16) * global<s16>(0x00faa5c4) / 2048; }
s32 ScaledY(u32 b) { return static_cast<s32>(b & 0xffff) * global<s16>(0x00faa5c0) / 1536; }
bool Far(s32 d) { return (d < 0 ? -d : d) > 8; }

void EndDrag() {   // (as a left press to the camera)
    g_env->camera(0x10005, 1, 0);
    drag() = 0;
}

// The key maps camera, game and 0x78721c in turn until one answers; then the cheat map when the cheats are on.
void Key(u32(__fastcall *fire)(u8 *, void *, u32, u32), u32 a, u32 b) {
    const u32 key = (b >> 16) & 0xff;
    if (fire(gKeymap_787c14, nullptr, a, key) || fire(gKeymap_787a60, nullptr, a, key) || fire(gKeymap_78721c, nullptr, a, key)) return;
    u8 &guard = global<u8>(0x00fb34c8);
    u8 *const cheats = reinterpret_cast<u8 *>((uintptr_t)0x00fb3520);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->cheatsCtor(cheats);
        g_env->atExit(reinterpret_cast<void *>((uintptr_t)0x005d1750));
    }
    if (g_env->cheatsOn(cheats)) fire(gKeymap_78733c, nullptr, a, key);
}
}

void Window7cb2acHandler_SetEnv(const Window7cb2acHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Window7cb2ac_Handler(u8 *window, u32 message, u32 a, u32 b) {
    toCamera() = 1;
    if (!global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x39)) {
        if (a == 1 && message == 0x10005) g_env->capture(window);
        else if (a == 1 && message == 0x10004) g_env->release(window);
    } else if (message == 0x1e) {
        if (drag() && static_cast<s32>(g_env->clock() - global<u32>(0x007c2504)) > 0xc8) EndDrag();
    } else if (message == 0x10003) {
        if (drag() && (Far((static_cast<s32>(a) >> 16) - global<s16>(0x007b5344)) || Far(static_cast<s32>(a & 0xffff) - global<s16>(0x007b9774)))) EndDrag();
    } else if (message == 0x10005 && a == 1) {
        g_env->capture(window);
        MouseModes_EnsureHolder();
        if (u8 *const mode = global<u8 *>(0x007b05d8)) (*reinterpret_cast<void(__fastcall *const *const *)(u8 *)>(mode))[9](mode);
        drag() = 1;
        global<u32>(0x007c2504) = g_env->clock();
        toCamera() = 0;
        global<s16>(0x007b5344) = static_cast<s16>(b >> 16);
        global<s16>(0x007b9774) = static_cast<s16>(b);
    } else if (message == 0x10004 && a == 1) {
        g_env->release(window);
        if (drag()) {
            u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
            u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
            MouseModes_EnsureHolder();
            MouseMode_Set(mode);
            g_env->gameMode(0, 1);
            g_env->hide();
        }
        drag() = 0;
    }
    if (toCamera()) g_env->camera(message, a, b);
    switch (message) {
    case 0x14:
        gPanel7b9940_7cb2ac = nullptr;
        break;
    case 0x15:
        drag() = 0;
        over() = 0;
        break;
    case 0x1e:
        MouseModes_EnsureHolder();
        g_env->moved(global<s16>(0x00faa5c4) * static_cast<s16>(gGuisettings_faa540) / 2048, global<s16>(0x00faa5c0) * static_cast<s16>(gGuisettings_faa544) / 1536);
        if (!global<u32>(0x007a1c7c)) g_env->unhighlight();
        if (!gFocus_7c24c8 && over()) g_env->help(0);
        break;
    case 0x10001:
        gMousehelp_7c24f8 = nullptr;
        if (!gFocus_7c24c8) g_env->help(0);
        over() = 1;
        gMouseclick_7c24f4 = 0;
        break;
    case 0x10002:
        over() = 0;
        break;
    case 0x10004: {
        const u32 modifiers = g_env->modifiers();
        g_env->fireUp(gKeymap_787c14, nullptr, a - 0x10, modifiers);
        MouseModes_EnsureHolder();
        g_env->moved(ScaledX(b), ScaledY(b));
        MouseModes_EnsureHolder();
        g_env->button(0xf0 + (a != 0), 0, ScaledX(b), ScaledY(b), 0);
        break;
    }
    case 0x10005: {
        const u32 modifiers = g_env->modifiers();
        g_env->fireDown(gKeymap_787c14, nullptr, a - 0x10, modifiers);
        if (gFocus_7c24c8 || a == 2) break;
        if (a == 0) {
            g_env->help(0);
            if (!g_env->shown()) break;
        }
        MouseModes_EnsureHolder();
        g_env->button(0xf0 + (a != 0), 1, ScaledX(b), ScaledY(b), 0);
        break;
    }
    case 0x1000a:
        Key(g_env->fireDown, a, b);
        break;
    case 0x1000b:
        Key(g_env->fireUp, a, b);
        break;
    default:
        break;
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
