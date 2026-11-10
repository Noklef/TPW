#include "moneydisplayhandler.h"

#include <cstdint>

#include "../core/globals.h"
#include "moneydisplay.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const MoneyDisplayHandlerEnv kGameEnv = {&MoneyDisplay_Update, &Global81af4c_Set_52fb90, &Window_CallDefault};
const MoneyDisplayHandlerEnv *g_env = &kGameEnv;

}  // namespace

extern "C" {

void MoneyDisplayHandler_SetEnv(const MoneyDisplayHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl MoneyDisplay_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const MoneyDisplayHandlerEnv *const E = g_env;
    if (message == 0x15) {
        reinterpret_cast<u32 &>(gMoneydisplay_7cb2dc) = 0;
        gMoneyDisplay_Handler_7cb2e0 = 0;
    } else if (message == 0x1e) {
        const u32 count = gMoneyDisplay_Handler_7cb2e0;
        gMoneyDisplay_Handler_7cb2e0 = count + 1;
        if (count & 2) {
            E->update(window);
            gMoneyDisplay_Handler_7cb2e0 = 0;
        }
        E->setCost(0);
    }
    return E->callDefault(window, nullptr, message, a, b);
}
}
