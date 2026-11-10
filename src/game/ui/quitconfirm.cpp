// The answer of the quit message box: see quitconfirm.h.
#include "quitconfirm.h"

#include <cstdint>

#include "../core/options.h"
#include "dialog74fb20.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const QuitConfirmEnv kGameEnv = {Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), Setting_SetMode};
const QuitConfirmEnv *g_env = &kGameEnv;

}  // namespace

void QuitConfirm_SetEnv(const QuitConfirmEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl QuitConfirm_Answered(u32 answer) {
    if (answer) return;
    *reinterpret_cast<u32 *>(global<u8 *>(0x00f82884) + 0x14) = 2;
    u8 &guard = gQuitconfirm_fb34bc;
    u8 *const setting = reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(setting);
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    g_env->setMode(setting, nullptr, 1);
}
}
