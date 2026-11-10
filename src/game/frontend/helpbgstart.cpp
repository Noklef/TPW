// The help background started: see helpbgstart.h.
#include "helpbgstart.h"

#include <cstdint>
#include "../core/staticobjects.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T game(u32 address) { return reinterpret_cast<T>((uintptr_t)address); }

const HelpBgStartEnv kGameEnv = {game<void(__fastcall *)(u8 *)>(0x00492180), game<u32(__fastcall *)(u8 *)>(0x0066a016), game<u32(__fastcall *)(u8 *, void *, u32)>(0x0065d9dd), game<void(__cdecl *)(u8 *)>(0x006595b3)};
const HelpBgStartEnv *g_env = &kGameEnv;

}  // namespace

void HelpBgStart_SetEnv(const HelpBgStartEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __cdecl HelpBg_Start() {
    u8 *const self = reinterpret_cast<u8 *>((uintptr_t)&gStatic_7ca0c0);
    g_env->build(self);
    if (global<u16>((u32)(uintptr_t)&gStatic_7ca0c0 + 0xc)) {
        g_env->stop(self);
        g_env->setShown(global<u8 *>((u32)(uintptr_t)&gStatic_7ca0c0 + 0x18), nullptr, 0);
    }
    g_env->setPointer(self);
}
