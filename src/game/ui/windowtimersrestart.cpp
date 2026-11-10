// The window timers set going again: see windowtimersrestart.h.
#include "windowtimersrestart.h"

#include <cstdint>

#include "guimanager.h"
#include "windowtimers.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
const WindowTimersRestartEnv kGameEnv = {&GuiClock_Now};
const WindowTimersRestartEnv *g_env = &kGameEnv;
}  // namespace

void WindowTimersRestart_SetEnv(const WindowTimersRestartEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall WindowTimer_Restart(u8 *timer) { *reinterpret_cast<u32 *>(timer + 4) = g_env->now(); }

void __cdecl WindowTimers_RestartAll() {
    if (!gWindowtimers_faa638) return;
    gWindowtimers_faa638 = 0;
    for (u8 *timer = gWindowtimers_faa634; timer; timer = *reinterpret_cast<u8 **>(timer + 0x18)) WindowTimer_Restart(timer);
}
}
