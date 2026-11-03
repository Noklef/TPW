// The game panel object 0x7cc328's window handler: see panel7cc328handler.h.
#include "panel7cc328handler.h"

#include <cstdint>

#include "../sprite/script.h"
#include "../world/ridesweep.h"
#include "dialog7ca098.h"
#include "dialog7ca150.h"
#include "dialog7ca168.h"
#include "dialogs.h"
#include "menuactions.h"
#include "panel7cc218.h"
#include "panel7cc328.h"
#include "panel7cc3b0.h"
#include "thinggoto.h"
#include "window.h"
#include "window7059f0.h"
#include "window7059f0texts.h"
#include "windowtimers.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Panel7cc328HandlerEnv kGameEnv = {&WindowAt10_Message4, &Window7059f0_NthItemData, &Window7059f0_SetItemTexts, &Window_SetTimer, &Window_StartTimer, &Game_CancelAdvisor, &ThingGoto_Start, &Things_StepGroup, &Things_StepGroupBack, &Dialog7ca150_Open, &Dialog7ca168_Open, &Panel7cc218_EnterMoveMode,
                                        &Panel7cc328_ShowStaff, &SpriteScript_Kill, &ThingPanel_Handler_48d200, &Panel7cc3b0_Show};
const Panel7cc328HandlerEnv *g_env = &kGameEnv;

u8 *panel() { return reinterpret_cast<u8 *>((uintptr_t)0x007cc328); }

void UpdateRows() {
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc338), nullptr, 0x1e7b);
    for (s32 row = *reinterpret_cast<s32 *>(list + 0x150) - 1; row >= 0; --row) {
        u32 data;
        g_env->nth(list, nullptr, row, &data);
        const u32 value = static_cast<u32>(static_cast<__int64>(*reinterpret_cast<const float *>((uintptr_t)data + 0x1fc))) & 0xff;
        u32 texts[2];  // (the first not set: only the second's bit is in the mask)
        texts[1] = static_cast<s32>(value << 10) / 100;
        g_env->setTexts(list, nullptr, row, texts, 2, row == 0 ? 1 : 0, 0xffffffff);
    }
}

}  // namespace

void Panel7cc328Handler_SetEnv(const Panel7cc328HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Panel7cc328_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u32 data;
    switch (message) {
    case 5:
        g_env->close(panel());
        return 1;
    case 0x10:
        if (a == 0x80080) UpdateRows();
        break;
    case 0x14:
        g_env->cancel(0x12c);
        break;
    case 0x15:
        if (static_cast<u16>(g_env->setTimer(window, nullptr, 0x80080, 0x7d0)) != 0) g_env->startTimer(window, nullptr, 0x80080);
        break;
    case 0x100:
        switch (a) {
        case 0xffffffff:
            g_env->close(panel());
            return 0;
        case 0x1e7a:
            g_env->gotoThing(panel());
            return 0;
        case 0x1e83:
            g_env->staffList(global<u32>(0x007cc358), 0);
            return 0;
        case 0x1e85:
            g_env->stepBack(panel());
            break;
        case 0x1e86:
            g_env->step(panel());
            break;
        case 0x1e87:
            g_env->moveMode(panel());
            return 0;
        case 0x1e88:
            g_env->jobs(3, global<u32>(0x007c2658));
            return 0;
        default:
            break;
        }
        break;
    case 0x400: {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {
            g_env->nth(child, nullptr, static_cast<s32>(b), &data);
            g_env->show(reinterpret_cast<u8 *>((uintptr_t)0x007cc3b0), nullptr, reinterpret_cast<u8 *>((uintptr_t)data));
        }
        return 0;
    }
    case 0x401: {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (!child) break;
        if (b != 0xffffffff) {
            g_env->nth(child, nullptr, static_cast<s32>(b), &data);
            g_env->showStaff(panel(), nullptr, reinterpret_cast<const u8 *>((uintptr_t)data));
        } else if (const s32 script = global<s32>(0x007cc34c)) {
            g_env->kill(script);
            global<u32>(0x007cc34c) = 0;
            global<u32>(0x007cc350) = 0xffffffff;
            global<u32>(0x007cc354) = 0xffffffff;
        }
        break;
    }
    case 0x406:
        global<u32>(0x00756f40) = b;
        break;
    default:
        break;
    }
    return g_env->base(window, message, a, b);
}
}
