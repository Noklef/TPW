// The thing panels' handlers: see thingpanelhandlers.h.
#include "thingpanelhandlers.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../world/ridesweep.h"
#include "dialog7ca098.h"
#include "dialog7ca150.h"
#include "dialogs.h"
#include "menuactions.h"
#include "panel7cc218.h"
#include "panel7cc258.h"
#include "panel7cc298.h"
#include "signrect.h"
#include "thinggoto.h"
#include "window.h"
#include "windowtimers.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl ForEach(u32 mask, u32 callback) { return Things_ForEachSelected(mask, reinterpret_cast<ObjectCallback>((uintptr_t)callback)); }

const ThingPanelHandlerEnv kGameEnv = {&WindowAt10_Message4, &Panel7cc258_ShowStats, &Panel7cc298_ShowStats, &ThingPanel_Handler_48d200, &Window_SetTimer, &Window_StartTimer, &Game_CancelAdvisor, &Sign_Swing, &Dialog7ca150_Open, &Things_StepGroup, &Things_StepGroupBack, &ThingPanel_RestartOrClose_48ccf0, &ThingGoto_Start, &Panel7cc218_EnterMoveMode, &ForEach};
const ThingPanelHandlerEnv *g_env = &kGameEnv;

u8 *self() { return reinterpret_cast<u8 *>((uintptr_t)0x007cc258); }
u8 *other() { return reinterpret_cast<u8 *>((uintptr_t)0x007cc298); }

void startStatsTimer(u8 *window) {
    if ((u16)g_env->setTimer(window, nullptr, 0x80080, 0xfa0)) g_env->startTimer(window, nullptr, 0x80080);
}

}  // namespace

void ThingPanelHandlers_SetEnv(const ThingPanelHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Panel7cc258_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        g_env->close(self());
        return 1;
    case 0x10:
        if (a == 0x80080) g_env->showStats258(self());
        break;
    case 0x14:
        g_env->cancelAdvisor(0xd1);
        break;
    case 0x15:
        startStatsTimer(window);
        break;
    case 0x1e:
        g_env->swing();
        break;
    case 0x800:
        if (a == 0xc06b) {
            global<u32>(0x007cc27c) = b;
            vcall<void>(Window_FindChild(global<u8 *>(0x007cc268), nullptr, 0xc07f), 0x1c, 1u << (b & 0x1f));
            vcall<void>(self(), 0x14);
        } else if (a == 0xc085) {
            global<u32>(0x007cc284) = b;
            vcall<void>(self(), 0x14);
        } else if (a == 0xc086) {
            global<u32>(0x007cc280) = b;
            vcall<void>(self(), 0x14);
        }
        break;
    case 0x100:
        switch (a) {
        case 0xfffffffe:
        case 0xffffffff:
            g_env->close(self());
            return 0;
        case 0xc067:
            g_env->open7ca150(1, global<u32>(0x007c2658));
            return 0;
        case 0xc068:
            g_env->goTo(self());
            return 0;
        case 0xc069:
            g_env->stepBack(self());
            break;
        case 0xc06a:
            g_env->stepGroup(self());
            break;
        case 0xc080:
            g_env->moveMode(self());
            return 0;
        case 0xc081:
            g_env->forEach(0x200, 0x004b0ae0);
            vcall<void>(self(), 0x14);
            return 0;
        case 0xc082:
            g_env->restart(self(), nullptr, b == 1 ? b : 0);
            g_env->showStats258(self());
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return g_env->base(window, message, a, b);
}

extern "C" s32 __cdecl Panel7cc298_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        g_env->close(other());
        return 1;
    case 0x10:
        if (a == 0x80080) g_env->showStats298(other());
        break;
    case 0x14:
        g_env->cancelAdvisor(0xd0);
        break;
    case 0x15:
        startStatsTimer(window);
        break;
    case 0x1e:
        g_env->swing();
        break;
    case 0x800:
        if (a == 0xa08e) {
            global<u32>(0x007cc2bc) = b;
            vcall<void>(other(), 0x14);
        } else if (a == 0xa08f) {
            global<u32>(0x007cc2c4) = b;
            vcall<void>(other(), 0x14);
        } else if (a == 0xa090) {
            global<u32>(0x007cc2c0) = b;
            vcall<void>(other(), 0x14);
        }
        break;
    case 0x100:
        switch (a) {
        case 0xfffffffe:
        case 0xffffffff:
            g_env->close(other());
            return 0;
        case 0xa089:
            g_env->open7ca150(2, global<u32>(0x007c2658));
            return 0;
        case 0xa08a:
            g_env->restart(other(), nullptr, b == 1 ? b : 0);
            g_env->showStats298(other());
            break;
        case 0xa08b:
            g_env->goTo(other());
            return 0;
        case 0xa08c:
            g_env->stepBack(other());
            break;
        case 0xa08d:
            g_env->stepGroup(other());
            break;
        case 0xa0a8:
            g_env->moveMode(other());
            return 0;
        case 0xa0a9:
            g_env->forEach(0x100, 0x004b20f0);
            vcall<void>(other(), 0x14);
            return 0;
        default:
            break;
        }
        break;
    default:
        break;
    }
    return g_env->base(window, message, a, b);
}
