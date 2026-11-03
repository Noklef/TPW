#include "dialog74fb20.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/keymap.h"
#include "focus.h"
#include "guimanager.h"
#include "guirect.h"
#include "window.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog74fb20Env kGameEnv = {Global7cb2e8_Get, Overlay7cb2b4_Toggle, WindowScript_Build, Window_GetRect, Window_SetWordsE4, Window_RunScript, Window_SetFocus, GuiClock_Now, Window_SendMessage,
                                  reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x0042a190), KeyMaps_EnableAll, Window_CallDefault};
const Dialog74fb20Env *g_env = &kGameEnv;

u8 *build() { return g_env->build(global<u8 *>(0x007cb2b4), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x0074fb20), 0x0048a740); }

// Moved to half its rectangle's size, the script 0x74fc38 run.
void place(u8 *window, u8 *rect) {
    g_env->getRect(window, nullptr, rect, 1);
    g_env->setWords(window, nullptr, (u32)(*reinterpret_cast<s16 *>(rect + 8) >> 1), (u32)(*reinterpret_cast<s16 *>(rect + 0xa) >> 1));
    g_env->runScript(window, nullptr, reinterpret_cast<u8 *>((uintptr_t)0x0074fc38), 0x23);
}

void closing(u8 *window) {
    g_env->send(window, 4, 0, 0);
    g_env->stopFollow();
    g_env->toggle();
}

}  // namespace

void Dialog74fb20_SetEnv(const Dialog74fb20Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog74fb20_Open() {
    global<u32>(0x007c24cc) = 0;
    if (!g_env->toggled()) g_env->toggle();
    u8 *const window = build();
    u8 rect[12];
    GuiRect_Ctor(rect);
    place(window, rect);
    g_env->setFocus(global<u8 *>(0x007cb2b4), nullptr, 0);
    GuiRect_Dtor(rect);
}

u32 __cdecl Dialog74fb20_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0x14:
        global<u32>(0x007c24cc) = 0;
        break;
    case 0x15:
        global<u32>(0x007c2508) = g_env->clock();
        break;
    case 0x10006:
        if (a == 1 && global<u8>(0x0078d911) && (s32)(g_env->clock() - global<u32>(0x007c2508)) > 0x7d0 && global<u32>(0x007c24cc)) {
            closing(window);
            u8 *const dialog = build();
            g_env->enableKeys();
            u8 rect[12];
            global<u8>(0x007c24d0) = 0;
            GuiRect_Ctor(rect);
            place(dialog, rect);
            GuiRect_Dtor(rect);
            return 0;
        }
        break;
    case 0x1000b:
        if (global<u32>(0x007c24cc) && a == 0x1b && !(b & 0xff0000)) {
            closing(window);
            u8 *const dialog = build();
            u8 rect[12];
            GuiRect_Ctor(rect);
            place(dialog, rect);
            g_env->enableKeys();
            global<u8>(0x007c24d0) = 0;
            GuiRect_Dtor(rect);
        }
        return 0;
    default:
        break;
    }
    return (u32)g_env->callDefault(window, nullptr, message, a, b);
}
}
