#include "dialog7ca1bc.h"

#include <cstdint>

#include "dialog7cb25c.h"
#include "dialog7cb2b8.h"
#include "dialog7cc2ec.h"
#include "dialog7cc444.h"
#include "dialogs.h"
#include "window.h"
#include "windowscript.h"
#include "../core/staticobjects.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7ca1bcHandlerEnv kGameEnv = {&Dialog7cc444_Create, &Dialog7cb25c_Open, &Dialog7cc2ec_Open, &Dialog7cb2b8_ResetChild21, &Window_CallDefault};
const Dialog7ca1bcHandlerEnv *g_env = &kGameEnv;
}

void Dialog7ca1bc_SetHandlerEnv(const Dialog7ca1bcHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7ca1bc_Create() {
    global<u8 *>((u32)(uintptr_t)&gStatic_7ca178 + 0x44) = WindowScript_Build(global<u8 *>(0x00faa5d8), nullptr, gDialog7ca1bc_Create_7514c0, 0x00497a70);
    Window_Unlink(global<u8 *>((u32)(uintptr_t)&gStatic_7ca178 + 0x44));
    Window_SetDwordA0(global<u8 *>((u32)(uintptr_t)&gStatic_7ca178 + 0x44), nullptr, 0, 0);
    Window_SetShown(global<u8 *>((u32)(uintptr_t)&gStatic_7ca178 + 0x44), nullptr, 0);
}

s32 __cdecl Dialog7ca1bc_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        global<u8 *>((u32)(uintptr_t)&gStatic_7ca178 + 0x44) = nullptr;
    } else if (message == 0x100 && (a == 0x1b59 || a == 0x1b5a || a == 0x1b5b)) {
        if (a == 0x1b59) g_env->open7cc444();
        else if (a == 0x1b5a) g_env->open7cb25c();
        else g_env->open7cc2ec();
        g_env->resetChild21(1);
    }
    return g_env->fallback(window, nullptr, message, a, b);
}
}
