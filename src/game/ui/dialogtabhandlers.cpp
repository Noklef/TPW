// The tabbed dialogs' handlers: see dialogtabhandlers.h.
#include "dialogtabhandlers.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/vcall.h"
#include "../world/parkopen.h"
#include "../world/world.h"
#include "dialog7ca098.h"
#include "dialog7ca210.h"
#include "dialog7ca2e8.h"
#include "dialog7cb25c.h"
#include "dialog7cb398.h"
#include "dialog7cc2ec.h"
#include "dialogbase.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const DialogTabHandlersEnv kGameEnv = {&Window_SendMessage, &Dialog_KeyHandler, &Game_CancelAdvisor, &GameTable7cfb90_Get1da726_5195d0, &Obj_Set118, &Game_OpenPark, &Dialog7ca2e8_Open, &Dialog7cb25c_Open, &Dialog7cc2ec_Open, &Dialog7ca210_Open, &Dialog7cb398_Create, &GameTable7cfb90_Get1da718_519450};
const DialogTabHandlersEnv *g_env = &kGameEnv;

u8 *game() { return gWorldPtr; }
void close(u32 dialog) { g_env->send(global<u8 *>(dialog), 4, 0, 0); }
s32 *budget() { return reinterpret_cast<s32 *>((uintptr_t)g_env->money(game()) + 0xc); }

// The budget's figures after one of its dwords changed: the sum and what is left.
void showBudget() {
    u8 *const figures = Window_FindChild(gDialog7cc2ec_7cc2ec, nullptr, 0x2de);
    if (!figures) return;
    s32 total = 0;
    for (u32 i = 0; i < 5; ++i) total += budget()[i];
    vcall<void>(Window_FindChild(figures, nullptr, 0x2df), 0x1c, total);
    vcall<void>(Window_FindChild(figures, nullptr, 0x2e4), 0x1c, global<s32>(0x007cc2d4) - total - global<s32>(0x007cc2e4));
}

}  // namespace

void DialogTabHandlers_SetEnv(const DialogTabHandlersEnv *env) { g_env = env ? env : &kGameEnv; }

s32 __cdecl Dialog7ca210_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        close(reinterpret_cast<uintptr_t>(&gDialog7ca210_7ca210));
        return 1;
    }
    if (message == 0x14) {
        gDialog7ca210_7ca210 = nullptr;
        g_env->set118(reinterpret_cast<u8 *>((uintptr_t)g_env->table(game())), nullptr, gDialog7ca210_7ca1e8);
        g_env->cancelAdvisor(0x12f);
    } else if (message == 0x800) {
        gDialog7ca210_7ca1e8 = b;
    } else if (message == 0x100) {
        switch (a) {
        case 0xfffffffe:
        case 0xffffffff: close(reinterpret_cast<uintptr_t>(&gDialog7ca210_7ca210)); return 0;
        case 0x4f3ab: g_env->open7ca2e8(); return 0;
        case 0x4f3ac: g_env->open7cb25c(); return 0;
        case 0x4f3ad: g_env->open7cc2ec(); return 0;
        case 0x4f3b1: g_env->openPark(game(), nullptr, b != 1, 0); break;
        }
    }
    return g_env->keys(window, message, a, b);
}

s32 __cdecl Dialog7cc2ec_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        close(reinterpret_cast<uintptr_t>(&gDialog7cc2ec_7cc2ec));
        return 1;
    }
    if (message == 0x14) {
        gDialog7cc2ec_7cc2ec = nullptr;
    } else if (message == 0x100) {
        switch (a) {
        case 0xffffffff: close(reinterpret_cast<uintptr_t>(&gDialog7cc2ec_7cc2ec)); return 0;
        case 0x2db: g_env->open7ca210(); return 0;
        case 0x2dc: g_env->open7ca2e8(); return 0;
        case 0x2dd: g_env->open7cb25c(); return 0;
        }
    } else if (message == 0x800) {
        s32 index = -1;
        switch (a) {
        case 0x2e7: index = 0; break;
        case 0x2ec: index = 1; break;
        case 0x2ea: index = 2; break;
        case 0x2eb: index = 3; break;
        case 0x2ed: index = 4; break;
        }
        if (index >= 0) {
            s32 *const figures = budget();
            const s32 value = static_cast<s32>(b) < 0 ? 0 : static_cast<s32>(b);
            figures[index] = value;
            if (value > 0) reinterpret_cast<u8 *>(figures)[0x14] = 1;
        }
        showBudget();
    }
    return g_env->keys(window, message, a, b);
}

s32 __cdecl Dialog7cc444_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        close(0x007cc444);
        return 1;
    }
    if (message == 0x14) {
        global<u8 *>(0x007cc444) = nullptr;
    } else if (message == 0x100) {
        if (a == 0xfffffffe) {
            close(0x007cc444);
            return 0;
        }
        if (a == 0xadb4) {
            g_env->open7cb398();
            return 0;
        }
        if (a == 0xadb5) {
            g_env->open7ca2e8();
            return 0;
        }
    }
    return g_env->keys(window, message, a, b);
}
