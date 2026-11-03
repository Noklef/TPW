// A dialog group's last tab opened again: see dialogtabs.h.
#include "dialogtabs.h"

#include <cstdint>

#include "dialog7ca13c.h"
#include "dialog7ca150.h"
#include "dialog7ca168.h"
#include "dialog7ca210.h"
#include "dialog7ca2e8.h"
#include "dialog7ca308.h"
#include "dialog7cb25c.h"
#include "dialog7cb398.h"
#include "dialog7cc1f8.h"
#include "dialog7cc2ec.h"
#include "focus.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const DialogTabsEnv kGameEnv = {&Global7c2534_HasSource, &Dialog7cc1f8_Open, &Dialog7ca308_Open, &Dialog7cb398_Create, &Dialog7ca168_Open, &Dialog7ca150_Open, &Dialog7ca13c_Open, &Dialog7ca2e8_Open, &Dialog7cb25c_Open, &Dialog7cc2ec_Open, &Dialog7ca210_Open};
const DialogTabsEnv *g_env = &kGameEnv;

}  // namespace

void DialogTabs_SetEnv(const DialogTabsEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl DialogTabs_Reopen(s32 group) {
    if (g_env->hasSource()) return;
    if (group == 1) {
        const s32 tab = global<s32>(0x007cb28c);
        if (tab == 1) g_env->open7cc1f8(0xffffffff, 0);
        else if (tab == 2) g_env->open7ca308(0xffffffff);
    } else if (group == 2) {
        switch (global<s32>(0x007cb2a0)) {
        case 3: g_env->open7cb398(); break;
        case 4: g_env->open7ca168(0xffffffff, 0); break;
        case 5: g_env->open7ca150(-1, 0); break;
        case 6: g_env->open7ca13c(0); break;
        }
    } else if (group == 3) {
        switch (global<s32>(0x007cb290)) {
        case 7: g_env->open7ca2e8(); break;
        case 8: g_env->open7cb25c(); break;
        case 9: g_env->open7cc2ec(); break;
        case 10: g_env->open7ca210(); break;
        }
    }
}
