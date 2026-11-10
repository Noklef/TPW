// The handler of the dialog [0x7cca70]: see dialog7cca70handler.h.
#include "dialog7cca70handler.h"

#include <cstdint>

#include "../frontend/scenequeue.h"
#include "../online/session.h"
#include "dialog7cc984open.h"
#include "window.h"
#include "window7059f0.h"
#include "dialog7cca70.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cca70HandlerEnv kGameEnv = {&Window_SetFocus, reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005b6070), &Class702da0_Clear, &Window_SendMessage, &Window7059f0_NthItemData, &Dialog7cc984_Open, &Window_CallDefault};
const Dialog7cca70HandlerEnv *g_env = &kGameEnv;

void close() {
    g_env->leaveSession(Online_Instance());
    g_env->clearScenes(at<u8 *>(global<u8 *>(0x00f85614), 0x14));
    g_env->send(global<u8 *>(0x007cca70), 4, 0, 0);
    if (void(__cdecl *const done)() = global<void(__cdecl *)()>(reinterpret_cast<uintptr_t>(&gDialog7cca70_7cca8c))) done();
}

}  // namespace

void Dialog7cca70Handler_SetEnv(const Dialog7cca70HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cca70_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u8 *>(0x007cca70) = nullptr;
        g_env->focus(at<u8 *>(global<u8 *>(0x00f82884), 4), nullptr, 0);
    } else if ((message == 0x100 && a == 0xfffffffe) || (message == 0x1000b && a == 0x1b)) {
        close();
        return 0;
    } else if (message == 0x400) {
        u8 *const list = Window_FindChild(window, nullptr, a);
        if (list && b != 0xffffffff) {
            u32 data;
            g_env->nth(list, nullptr, static_cast<s32>(b), &data);
            g_env->open(reinterpret_cast<const u8 *>((uintptr_t)data), window, 0, 0x004c0830, 0);
        }
    }
    return g_env->handler(window, nullptr, message, a, b);
}
}
