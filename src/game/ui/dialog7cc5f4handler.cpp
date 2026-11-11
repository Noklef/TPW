// The dialog [0x7cc5f4]'s handler: see dialog7cc5f4handler.h.
#include "dialog7cc5f4handler.h"

#include <cstdint>

#include "../core/keymap.h"
#include "../online/session.h"
#include "dialogbase.h"
#include "focus.h"
#include "window.h"
#include "dialog7cc5f4dial.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc5f4HandlerEnv kGameEnv = {Window_SendMessage, Window_PostMessage, Focus_Restore, KeyMaps_EnableAllAndClearFlag, KeyMaps_ResetAllAndSetFlag, Online_Instance, reinterpret_cast<void(__fastcall *)(u8 *, void *, u8 *, u8 *)>((uintptr_t)0x005b5d10), Window_FindChild, Window_SetFocus, Dialog_BaseHandler};
const Dialog7cc5f4HandlerEnv *g_env = &kGameEnv;

u8 *mainWindow() { return at<u8 *>(global<u8 *>(0x00f82884), 4); }

s32 closed() {
    g_env->send(global<u8 *>(0x007cc5f4), 4, 0, 0);
    if (void(__cdecl *const callback)() = global<void(__cdecl *)()>(0x007cc5fc)) callback();
    return 1;
}

}  // namespace

void Dialog7cc5f4_SetHandlerEnv(const Dialog7cc5f4HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cc5f4_Message(u8 *window, u32 message, u32 a, u32) {
    switch (message) {
    case 0x14:
        global<u32>(0x007cc5f4) = 0;
        g_env->send(mainWindow(), 6, 1, 0);
        g_env->restoreFocus();
        g_env->enableKeys();
        break;
    case 0x15:
        g_env->send(mainWindow(), 6, 0, 0);
        g_env->resetKeys();
        break;
    case 0x100:
        if (a == 0xfffffffe) return closed();
        if (a == 0xffffffff) g_env->apply(g_env->online(), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007cc590), gDialog7cc5f4dial_7cc548);
        break;
    case 0x802:
        if (a == 0x49) g_env->focus(g_env->find(window, nullptr, 0x4b), nullptr, 0);
        else g_env->post(window, 0x100, 0xffffffff, 0);
        break;
    case 0x803:
        g_env->focus(g_env->find(window, nullptr, a == 0x49 ? 0x4b : 0x49), nullptr, 0);
        break;
    case 0x804:
        return closed();
    default:
        break;
    }
    return 0;
}

s32 __cdecl Dialog7cc5f4_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (Dialog7cc5f4_Message(window, message, a, b)) return 0;
    return g_env->base(window, message, a, b);
}
}
