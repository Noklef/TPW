// The GUI's activation told to the window tree: see windowbroadcast.h.
#include "windowbroadcast.h"

#include <cstdint>

#include "../core/accessors.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const WindowBroadcastEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u8 *, u32, u32, u32)>((uintptr_t)0x00658ccc)};
const WindowBroadcastEnv *g_env = &kGameEnv;
}  // namespace

void WindowBroadcast_SetEnv(const WindowBroadcastEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl WindowTree_PostActive(u8 *window, u32 on) {
    g_env->post(window, on ? 0x21 : 0x22, 0, 0);
    u8 *child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d547(window, nullptr, 0));
    while (child) {
        WindowTree_PostActive(child, on);
        child = reinterpret_cast<u8 *>((uintptr_t)Fn_65d56b(child));
    }
}

void __cdecl Gui_TellActive() {
    if (global<u8 *>(0x00faa5d8) && !gWindowbroadcast_77c488) {
        WindowTree_PostActive(global<u8 *>(0x00faa5d8), 1);
        gWindowbroadcast_77c488 = 1;
    }
}

void __cdecl Gui_TellInactive() {
    if (global<u8 *>(0x00faa5d8) && gWindowbroadcast_77c488) {
        WindowTree_PostActive(global<u8 *>(0x00faa5d8), 0);
        gWindowbroadcast_77c488 = 0;
    }
    global<u16>(0x00faa5bc) = 0;
}
}
