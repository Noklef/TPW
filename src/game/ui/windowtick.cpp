#include "windowtick.h"

#include <cstdint>

#include "window.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const WindowTickEnv kGameEnv = {&Window_SendMessage};
const WindowTickEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void WindowTick_SetEnv(const WindowTickEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl WindowTick_Tick(u8 *window) {
    g_env->send(window, 0x1e, 0, 0);
    ++gWindowTick_faa60c;
    for (u8 *child = at<u8 *>(window, 0x5c); child; child = at<u8 *>(child, 0x54)) WindowTick_Tick(child);
}

u32 __cdecl WindowTick_TickAll() {
    gWindowTick_faa60c = 0;
    if (global<u8 *>(0x00faa5d8)) WindowTick_Tick(global<u8 *>(0x00faa5d8));
    return gWindowTick_faa60c;
}
}
