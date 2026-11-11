// The window's name: see windowname.h.
#include "windowname.h"

#include <cstdint>
#include "../core/addrcalls.h"

namespace {
const WindowNameEnv kGameEnv = {GameAddress<void *(__cdecl *)(void *, const void *, u32)>(0x0067b0d0),
                                GameAddress<void *(__cdecl *)(void *, const void *)>(0x0067bf10)};
const WindowNameEnv *g_env = &kGameEnv;
}  // namespace

void WindowName_SetEnv(const WindowNameEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Window_SetName(u8 *self, void *, const void *name) { g_env->wcsncpy(self + 0x6c, name, 0xf); }

void __fastcall Window_GetName(u8 *self, void *, void *out) {
    if (out) g_env->wcscpy(out, self + 0x6c);
}
}
