#include "gamewindowinput.h"

#include <cstdint>

#include "../core/keymap.h"
#include "../world/camera.h"
#include "focus.h"
#include "panel7c2534menus.h"
#include "window.h"
#include "../world/world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const GameWindowInputEnv kGameEnv = {
    &Window_CaptureMouse,
    &Window_ReleaseMouse,
    reinterpret_cast<void(__cdecl *)(u32, u32, u32)>(&Camera_Input),
    &KeyMap_Modifiers,
    &Actions_FireUp,
    &Actions_FireDown,
    &Window_CallDefault,
    &Panel7c2534_Open,
    reinterpret_cast<s32(__fastcall *)(const u8 *, void *, u32, u32)>(&KeyMap_FindKey),
    &Overlay7cb2b4_Toggle,
    reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x004a9380),
};
const GameWindowInputEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void GameWindowInput_SetEnv(const GameWindowInputEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl GameWindow_Input(u8 *window, u32 message, u32 a, u32 b) {
    const GameWindowInputEnv *const E = g_env;
    if (a == 1) {
        if (message == 0x10004)
            E->release(window);
        else if (message == 0x10005)
            E->capture(window);
    }
    E->camera(message, a, b);
    const u32 mods = (b >> 16) & 0xff;
    switch (message) {
        case 0x10004: {
            const u32 held = E->modifiers();
            E->fireUp(gKeymap_787c14, nullptr, a - 0x10, held);
            break;
        }
        case 0x10005: {
            const u32 held = E->modifiers();
            E->fireDown(gKeymap_787c14, nullptr, a - 0x10, held);
            break;
        }
        case 0x1000a:
            E->fireDown(gKeymap_787c14, nullptr, a, mods);
            break;
        case 0x1000b: {
            E->fireUp(gKeymap_787c14, nullptr, a, mods);
            const bool plainEscape = a == 0x1b && !(b & 0xff0000);
            if (at<u32>(gWorldPtr, 0x1da738) == 4) {
                if (plainEscape) E->openPanel(0);
                return 0;
            }
            if (static_cast<u16>(E->findKey(gKeymap_787a60, nullptr, a, mods)) == 4 || plainEscape) {
                E->toggleOverlay();
                return 0;
            }
            if (static_cast<u16>(E->findKey(gKeymap_78721c, nullptr, a, mods)) == 0xf) E->key15();
            return 0;
        }
        default:
            break;
    }
    return E->callDefault(window, nullptr, message, a, b);
}
}
