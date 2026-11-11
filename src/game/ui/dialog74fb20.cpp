#include "dialog74fb20.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/keymap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../world/camera.h"
#include "../world/object.h"
#include "focus.h"
#include "guimanager.h"
#include "guirect.h"
#include "window.h"
#include "windowfactory.h"
#include "windowscript.h"
#include "panel7b9940.h"
#include "../world/world.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog74fb20Env kGameEnv = {Global7cb2e8_Get, Overlay7cb2b4_Toggle, WindowScript_Build, Window_GetRect, Window_SetWordsE4, Window_RunScript, Window_SetFocus, GuiClock_Now, Window_SendMessage,
                                  Camera_StopFollowing, KeyMaps_EnableAll, Window_CallDefault, Camera_ToggleWalkMode,
                                  reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32)>(&Object_FollowCamera), KeyMaps_ResetAll, Focus_Restore, Window_CreateChild, Window_SetHandler, Setting_ReadMode,
                                  GameAddress<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0), GameAddress<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>(0x0047f020),
                                  GameAddress<void(__fastcall *)(u8 *)>(0x00516c10)};
const Dialog74fb20Env *g_env = &kGameEnv;

u8 *build() { return g_env->build(gFocus_7cb2b4, nullptr, gDialog74fb20_74fb20, 0x0048a740); }

// Moved to half its rectangle's size, the script run.
void place(u8 *window, u8 *rect, u32 script = 0x0074fc38) {
    g_env->getRect(window, nullptr, rect, 1);
    g_env->setWords(window, nullptr, (u32)(*reinterpret_cast<s16 *>(rect + 8) >> 1), (u32)(*reinterpret_cast<s16 *>(rect + 0xa) >> 1));
    g_env->runScript(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)script), 0x23);
}

void closing(u8 *window) {
    g_env->send(window, 4, 0, 0);
    g_env->stopFollow();
    g_env->toggle();
}

// The camera mode left, the bars hidden, a host window made for the dialog.
u8 *hide(u8 *thing) {
    CameraMode_Leave();
    g_env->thingOp(thing, nullptr, 0);
    g_env->resetKeys();
    gDialog74fb20_7c24d0 = 1;
    if (g_env->toggled()) g_env->toggle();
    if (gFocus_7c24c8) {
        g_env->send(gFocus_7c24c8, 5, 0, 0);
        gFocus_7c24c8 = nullptr;
        g_env->restore();
    }
    g_env->send(gPanel7b9940_7cb2ac, 6, 0, 0);
    g_env->send(gPanel7b9940_7cb2b0, 6, 0, 0);
    return g_env->create(global<u8 *>(0x00faa5d8), nullptr, 1, 1, 0x7472, 0, 0, 0x7ff, 0x5ff);
}

// Dialog74faf0's way out: the bars back, and in the mode 2 the box of the text 0x1d7.
void closingB(u8 *window, u32 bar) {
    g_env->send(window, 4, 0, 0);
    g_env->tableOp(gWorldPtr);
    g_env->send(gPanel7b9940_7cb2b0, 6, 0, 0);
    g_env->send(gPanel7b9940_7cb2ac, 6, bar, 0);
    g_env->enableKeys();
    gDialog74fb20_7c24d0 = 0;
    g_env->restore();
    u8 &guard = gDialog74fb20_fb34c7;
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c != 2) return;
    struct {
        u16 *text;
        u8 colour[4];   // (its last byte never written, sic)
        u32 kind, data;
    } record;
    record.text = Language_WideText(0x1d7);
    record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
    record.kind = 4;
    record.data = 0;
    g_env->box(reinterpret_cast<const u8 *>(&record), 2, 0, 0, 0, 0, 0, 0);
}

}  // namespace

void Dialog74fb20_SetEnv(const Dialog74fb20Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog74fb20_Open() {
    gDialog74fb20_7c24cc = 0;
    if (!g_env->toggled()) g_env->toggle();
    u8 *const window = build();
    u8 rect[12];
    GuiRect_Ctor(rect);
    place(window, rect);
    g_env->setFocus(gFocus_7cb2b4, nullptr, 0);
    GuiRect_Dtor(rect);
}

u32 __cdecl Dialog74fb20_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0x14:
        gDialog74fb20_7c24cc = 0;
        break;
    case 0x15:
        gDialog74fb20_7c2508 = g_env->clock();
        break;
    case 0x10006:
        if (a == 1 && global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x39) && (s32)(g_env->clock() - gDialog74fb20_7c2508) > 0x7d0 && gDialog74fb20_7c24cc) {
            closing(window);
            u8 *const dialog = build();
            g_env->enableKeys();
            u8 rect[12];
            gDialog74fb20_7c24d0 = 0;
            GuiRect_Ctor(rect);
            place(dialog, rect);
            GuiRect_Dtor(rect);
            return 0;
        }
        break;
    case 0x1000b:
        if (gDialog74fb20_7c24cc && a == 0x1b && !(b & 0xff0000)) {
            closing(window);
            u8 *const dialog = build();
            u8 rect[12];
            GuiRect_Ctor(rect);
            place(dialog, rect);
            g_env->enableKeys();
            gDialog74fb20_7c24d0 = 0;
            GuiRect_Dtor(rect);
        }
        return 0;
    default:
        break;
    }
    return (u32)g_env->callDefault(window, nullptr, message, a, b);
}

void __cdecl CameraMode_Leave() {
    const u32 flags = gCameraFlags;
    if (flags & 2) {
        g_env->walk();
    } else if (flags & 4) {
        g_env->stopFollow();
        if (flags & 0x400) g_env->walk();
    }
}

void __cdecl Dialog74fb20_OpenFor(u8 *thing) {
    u8 *const host = hide(thing);
    g_env->setHandler(host, nullptr, 0x0048a740);
    u8 *const dialog = g_env->build(host, nullptr, gDialog74fb20_74fb20, 0);
    u8 rect[12];
    GuiRect_Ctor(rect);
    place(dialog, rect);
    g_env->setFocus(host, nullptr, 0);
    reinterpret_cast<u8 * &>(gDialog74fb20_7c24cc) = host;
    GuiRect_Dtor(rect);
}

void __cdecl Dialog74faf0_OpenFor(u8 *thing) {
    u8 *const host = hide(thing);
    u8 *const dialog = g_env->build(host, nullptr, gDialog74faf0_OpenFor_74faf0, 0);
    vcall<void>(dialog, 0x14, 0u);
    u8 rect[12];
    GuiRect_Ctor(rect);
    place(dialog, rect, (u32)(uintptr_t)&gDialog74faf0_OpenFor_74fcf8);
    g_env->setHandler(host, nullptr, 0x0048a970);
    g_env->setFocus(host, nullptr, 0);
    GuiRect_Dtor(rect);
}

u32 __cdecl Dialog74faf0_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0x15:
        gDialog74fb20_7c2508 = g_env->clock();
        break;
    case 0x10006:
        if (a == 1 && global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x39) && (s32)(g_env->clock() - gDialog74fb20_7c2508) > 0x7d0) {
            closingB(window, a);
            return 0;
        }
        break;
    case 0x1000b:
        if (a == 0x1b && !(b & 0xff0000)) closingB(window, 1);
        return 0;
    default:
        break;
    }
    return (u32)g_env->callDefault(window, nullptr, message, a, b);
}
}
