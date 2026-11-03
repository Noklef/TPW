// The handler of the child 0x25 of the window [0x7cb2b8]: see dialog7cb2b8tabs.h.
#include "dialog7cb2b8tabs.h"

#include <cstdint>

#include "dialogtabs.h"
#include "levelmeter.h"
#include "menuactions.h"
#include "window.h"

namespace {

const Dialog7cb2b8TabsEnv kGameEnv = {DialogTabs_Reopen, Dialog7ca1e0_Post_498bb0, Dialog7ca1e0_Cancel_498bd0, reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x005f0b40), ResearchPanel_Open, Window_CallDefault};
const Dialog7cb2b8TabsEnv *g_env = &kGameEnv;

}  // namespace

void Dialog7cb2b8_SetTabsEnv(const Dialog7cb2b8TabsEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cb2b8_TabsHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x100) {
        switch (a) {
        case 0x26: g_env->reopen(1); break;
        case 0x27:
            if (b == 1) g_env->post();
            else g_env->cancel();
            break;
        case 0x28: g_env->reopen(2); break;
        case 0x29: g_env->other(reinterpret_cast<u8 *>((uintptr_t)0x00f86db8)); break;
        case 0x2a: g_env->reopen(3); break;
        case 0x2b: g_env->research(); break;
        default: break;
        }
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
}
