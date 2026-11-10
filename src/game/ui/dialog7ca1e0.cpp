#include "dialog7ca1e0.h"

#include <cstdint>

#include "../world/mousemode.h"
#include "dialog7ca098.h"
#include "dialogs.h"
#include "gamepause.h"
#include "window.h"
#include "windowscript.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7ca1e0Env kGameEnv = {&MouseMode_Enter6fead0, &GamePause_Hide, &Game_CancelAdvisor, &Dialog7cb2b8_ResetChild21, &Window_CallDefault};
const Dialog7ca1e0Env *g_env = &kGameEnv;
}

extern "C" {

void Dialog7ca1e0_SetEnv(const Dialog7ca1e0Env *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl Dialog7ca1e0_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const Dialog7ca1e0Env *const E = g_env;
    if (message == 0x14) {
        gDialog7ca1e0_7ca1e0 = nullptr;
    } else if (message == 0x100) {
        if (a == 0x63)
            E->enterMode();
        else if (a == 0x64)
            E->pauseHide();
        E->cancelAdvisor(0x131);
        E->resetChild21(1);
    }
    return E->callDefault(window, nullptr, message, a, b);
}

void __cdecl Dialog7ca1e0_Create() {
    gDialog7ca1e0_7ca1e0 = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, gDialog7ca1e0_Create_751720, 0x00498ad0);
    Window_Unlink(gDialog7ca1e0_7ca1e0);
    Window_SetDwordA0(gDialog7ca1e0_7ca1e0, nullptr, 0, 1);
    Window_SetShown(gDialog7ca1e0_7ca1e0, nullptr, 0);
}
}
