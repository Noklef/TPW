#include "window7cb2b0handler.h"

#include <cstdint>

#include "../core/keymap.h"
#include "../world/camera.h"
#include "cursor.h"
#include "gamepause.h"
#include "window.h"
#include "panel7b9940.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Window7cb2b0HandlerEnv kGameEnv = {
    &KeyMap_Modifiers,
    &Actions_FireUp,
    &Actions_FireDown,
    reinterpret_cast<s32(__fastcall *)(const u8 *, void *, u32, u32)>(&KeyMap_FindKey),
    &Window_SetCursor,
    &Camera_ToggleWalkMode,
    &Camera_StopFollowing,
    &GamePause_Hide,
    reinterpret_cast<void(__cdecl *)(u32, u32, u32)>(&Camera_Input),
    &Window_CallDefault,
};
const Window7cb2b0HandlerEnv *g_env = &kGameEnv;

u8 *Actions() { return gKeymap_787c14; }

void LetCameraGo(u8 *window) {
    const Window7cb2b0HandlerEnv *const E = g_env;
    if (!(reinterpret_cast<u8 &>(gCameraFlags) & 0x16)) return;
    E->setCursor(window, nullptr, 1);
    const u8 mode = reinterpret_cast<u8 &>(gCameraFlags);
    if (mode & 2)
        E->walk();
    else if (mode & 4)
        E->stopFollowing();
    E->setCursor(window, nullptr, 0x15);
}

}  // namespace

extern "C" {

void Window7cb2b0Handler_SetEnv(const Window7cb2b0HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl Window7cb2b0_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const Window7cb2b0HandlerEnv *const E = g_env;
    switch (message) {
        case 0x14:
            gPanel7b9940_7cb2b0 = nullptr;
            break;
        case 0x100:
            LetCameraGo(window);
            break;
        case 0x10004: {
            const u32 modifiers = E->modifiers();
            E->fireUp(Actions(), nullptr, a - 0x10, modifiers);
            break;
        }
        case 0x10005: {
            const u32 modifiers = E->modifiers();
            E->fireDown(Actions(), nullptr, a - 0x10, modifiers);
            break;
        }
        case 0x10006:
            if (global<u8>(0x78d911) && a == 1) LetCameraGo(window);
            break;
        case 0x1000a:
            E->fireDown(Actions(), nullptr, a, (b >> 16) & 0xff);
            break;
        case 0x1000b: {
            const u32 modifiers = (b >> 16) & 0xff;
            E->fireUp(Actions(), nullptr, a, modifiers);
            if (a == 0x1b || static_cast<u16>(E->findKey(gKeymap_78721c, nullptr, a, modifiers)) == 0x10)
                LetCameraGo(window);
            else if (static_cast<u16>(E->findKey(gKeymap_78721c, nullptr, a, modifiers)) == 0xf)
                E->pauseHide();
            break;
        }
        default:
            break;
    }
    E->camera(message, a, b);
    return E->callDefault(window, nullptr, message, a, b);
}
}
