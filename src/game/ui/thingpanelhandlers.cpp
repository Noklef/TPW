// The thing panels' handlers: see thingpanelhandlers.h.
#include "thingpanelhandlers.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/ridesweep.h"
#include "../world/world.h"
#include "dialog7ca098.h"
#include "dialog7ca13c.h"
#include "dialog7ca150.h"
#include "dialogs.h"
#include "menuactions.h"
#include "panel7ca178.h"
#include "panel7cc218.h"
#include "panel7cc258.h"
#include "panel7cc298.h"
#include "panel7cc408info.h"
#include "signrect.h"
#include "thingbrowser.h"
#include "thinggoto.h"
#include "thingpreview.h"
#include "window.h"
#include "windowtimers.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl ForEach(u32 mask, u32 callback) { return Things_ForEachSelected(mask, reinterpret_cast<ObjectCallback>((uintptr_t)callback)); }

void __fastcall SetFlag(u8 *panel, void *, u32 flag) { ThingPanel_SetFlag64_48ce50(panel, nullptr, (u8)flag); }

const ThingPanelHandlerEnv kGameEnv = {&WindowAt10_Message4, &Panel7cc258_ShowStats, &Panel7cc298_ShowStats, &ThingPanel_Handler_48d200, &Window_SetTimer, &Window_StartTimer, &Game_CancelAdvisor, &Sign_Swing, &Dialog7ca150_Open, &Things_StepGroup, &Things_StepGroupBack, &ThingPanel_RestartOrClose_48ccf0, &ThingGoto_Start, &Panel7cc218_EnterMoveMode, &ForEach, &Panel7ca178_ShowStats, &SetFlag, &Global7c24d8_CopyScript, &Panel7cc408_ShowVisitor, &ThingBrowser_Open, &Dialog7ca13c_Open, &Object_GetDef, &GameTable7cfb90_Get1da720_519510, &ObjectControls_GetWord18, &Object_ScrapRefund};
const ThingPanelHandlerEnv *g_env = &kGameEnv;

u8 *self() { return reinterpret_cast<u8 *>((uintptr_t)0x007cc258); }
u8 *other() { return reinterpret_cast<u8 *>((uintptr_t)0x007cc298); }

void startStatsTimer(u8 *window) {
    if ((u16)g_env->setTimer(window, nullptr, 0x80080, 0xfa0)) g_env->startTimer(window, nullptr, 0x80080);
}

}  // namespace

void ThingPanelHandlers_SetEnv(const ThingPanelHandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Panel7ca178_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)0x007ca178);
    switch (message) {
    case 5:
        g_env->close(panel);
        return 1;
    case 0x10:
        if (a == 0x80080) g_env->showStats178(panel);
        break;
    case 0x14:
        g_env->cancelAdvisor(0x12b);
        g_env->cancelAdvisor(0xd5);
        break;
    case 0x15:
        startStatsTimer(window);
        break;
    case 0x1e:
        g_env->swing();
        break;
    case 0x100:
        switch (a) {
        case 0xffffffff:
            g_env->close(panel);
            return 0;
        case 0x15bb4:
            g_env->setFlag(panel, nullptr, b == 1 ? 0 : 1);
            return 0;
        case 0x15bb5:
            g_env->goTo(panel);
            return 0;
        case 0x15bb6:
            g_env->stepBack(panel);
            break;
        case 0x15bb7:
            g_env->stepGroup(panel);
            break;
        case 0x15bc2:
            g_env->open7ca150(3, reinterpret_cast<u32 &>(gPanel7cc218_7c2658));
            return 0;
        case 0x15bc3:
            g_env->moveMode(panel);
            return 0;
        case 0x15bc4:
            g_env->restart(panel, nullptr, b == 1 ? 1 : 0);
            g_env->showStats178(panel);
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

extern "C" s32 __cdecl Panel7cc408_Handler(u8 *window, u32 message, u32 a, u32 b) {
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)0x007cc408);
    switch (message) {
    case 5:
        g_env->close(panel);
        return 1;
    case 0x10:
        if (a == 0x80080) g_env->showVisitor(panel);
        break;
    case 0x14:
        g_env->cancelAdvisor(0x12e);
        break;
    case 0x15:
        startStatsTimer(window);
        break;
    case 0x1e:
        g_env->copyScript();
        break;
    case 0x100:
        switch (a) {
        case 0xfffffffe:
            g_env->close(panel);
            return 0;
        case 0x3720:
            g_env->stepBack(panel);
            break;
        case 0x3721:
            g_env->stepGroup(panel);
            break;
        case 0x3737:
            g_env->openBrowser(gPanel7cc218_7c2658);
            return 0;
        case 0x3738:
            g_env->open7ca13c(reinterpret_cast<u32 &>(gPanel7cc218_7c2658));
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

extern "C" s32 __cdecl Panel7cc3e0_Handler(u8 *window, u32 message, u32 a, u32 b) {
    using SetValue = void(__fastcall *)(u8 *, void *, u32);
    u8 *const panel = reinterpret_cast<u8 *>((uintptr_t)0x007cc3e0);
    switch (message) {
    case 5:
        g_env->close(panel);
        return 1;
    case 0x10:
        if (a == 0x80080) {
            const u8 *const thing = gPanel7cc218_7c2658;
            g_env->def(thing);
            if (!global<u8 *>(0x007cc3f4)) break;
            if (u8 *const child = Window_FindChild(global<u8 *>(0x007cc3f4), nullptr, 0x5210)) {
                const SetValue set = (*reinterpret_cast<SetValue *const *>(child))[7];
                g_env->table(global<u8 *>(0x007cf83c));   // (its answer only the ECX of the stdcall)
                set(child, nullptr, g_env->control((a & 0xffff0000) | *reinterpret_cast<const u16 *>(thing + 0xe)));
            }
            u8 *const child = Window_FindChild(global<u8 *>(0x007cc3f4), nullptr, 0x520e);
            const SetValue set = (*reinterpret_cast<SetValue *const *>(child))[7];
            set(child, nullptr, g_env->refund(thing));
        }
        break;
    case 0x14:
        g_env->cancelAdvisor(0x12d);
        break;
    case 0x15:
        startStatsTimer(window);
        break;
    case 0x1e:
        g_env->swing();
        break;
    case 0x100:
        switch (a) {
        case 0xffffffff:
            g_env->close(panel);
            return 0;
        case 0x5209:
            g_env->goTo(panel);
            return 0;
        case 0x520a:
            g_env->stepBack(panel);
            break;
        case 0x520b:
            g_env->stepGroup(panel);
            break;
        case 0x5213:
            g_env->open7ca150(0, 0);
            return 0;
        case 0x5214:
            g_env->moveMode(panel);
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
            g_env->open7ca150(1, reinterpret_cast<u32 &>(gPanel7cc218_7c2658));
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
            g_env->open7ca150(2, reinterpret_cast<u32 &>(gPanel7cc218_7c2658));
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
