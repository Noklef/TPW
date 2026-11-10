// The window handler of the panel at 0x7cc3b0 and its box's answer: see panel7cc3b0handler.h.
#include "panel7cc3b0handler.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/idmaps.h"
#include "../core/language.h"
#include "../world/analysercell.h"
#include "../world/ridesweep.h"
#include "../world/staff.h"
#include "dialog7ca098.h"
#include "dialog7ca168.h"
#include "dialogs.h"
#include "menuactions.h"
#include "panel7cc3b0.h"
#include "thingbrowser.h"
#include "thingpreview.h"
#include "window.h"
#include "windowtimers.h"
#include "panel7cc218.h"
#include "../core/staticobjects.h"

namespace {

u8 *const kPanel = reinterpret_cast<u8 *>((uintptr_t)&gStatic_7cc3b0);

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *thing() { return gPanel7cc218_7c2658; }

void closePanel() { reinterpret_cast<void(__fastcall *)(u8 *)>((*reinterpret_cast<uintptr_t **>(kPanel))[11])(kPanel); }

const Panel7cc3b0HandlerEnv kGameEnv = {ThingPanel_Handler_48d200, WindowAt10_Message4, Game_CancelAdvisor, Window_SetTimer, Window_StartTimer, AnalyserRecord_ChecksCell, Global7c24d8_CopyScript, Panel7cc3b0_ShowStats, Byte2_ToIndex, Dialog7ca168_Open, Things_StepGroup, Things_StepGroupBack, Panel7cc3b0_SetPatrol, Language_WideText,
                                        reinterpret_cast<void(__cdecl *)(const u8 *, u32, u32, u32, u32, u32, u32, u32)>((uintptr_t)0x0047f020), Staff_Dismiss, Global7c2650_Set, Staff_PickUp, ThingBrowser_Open};
const Panel7cc3b0HandlerEnv *g_env = &kGameEnv;

}  // namespace

void Panel7cc3b0_SetHandlerEnv(const Panel7cc3b0HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Panel7cc3b0_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 5:
        g_env->message4(kPanel);
        return 1;
    case 0x10:
        if (a == 0x80080) g_env->showStats(kPanel);
        break;
    case 0x14:
        g_env->cancel(0xc0);
        g_env->cancel(0xd4);
        break;
    case 0x15:
        if ((u16)g_env->setTimer(window, nullptr, 0x80080, 0xfa0)) g_env->startTimer(window, nullptr, 0x80080);
        break;
    case 0x1e:
        if (thing()) {
            u8 *const child = Window_FindChild(global<u8 *>((u32)(uintptr_t)&gStatic_7cc3b0 + 0x10), nullptr, 0x50e);
            const u32 on = g_env->checksCell(thing()) ? 1 : 0;
            reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((*reinterpret_cast<uintptr_t **>(child))[5])(child, nullptr, on);
            g_env->copyScript();
        }
        break;
    case 0x100:
        switch (a) {
        case 0xffffffff:
            g_env->message4(kPanel);
            return 0;
        case 0x50b:
            g_env->openStaff(g_env->toIndex(thing()), (u32)(uintptr_t)thing());
            return 0;
        case 0x50c:
            if (global<u8>((u32)(uintptr_t)&gOptions_78d8d8 + 0x37)) {
                struct {
                    u16 *text;
                    u8 colour[4];
                    u32 kind;
                    u8 *data;
                } record;
                record.text = g_env->text(0x18d);
                record.colour[0] = record.colour[1] = record.colour[2] = 0xff;
                record.kind = 4;
                record.data = thing();
                g_env->box(reinterpret_cast<const u8 *>(&record), 2, 0, 1, 0x004b6070, 0, 0, 0);
            } else {
                g_env->dismiss(thing());
            }
            closePanel();
            return 0;
        case 0x50d:
            g_env->stepBack(kPanel);
            break;
        case 0x50e: {
            u8 *const picked = thing();
            g_env->setPicked((u32)(uintptr_t)picked);
            g_env->pickUp(picked);
            closePanel();
            return 0;
        }
        case 0x50f:
            g_env->stepGroup(kPanel);
            break;
        case 0x510:
            if (g_env->setPatrol(kPanel, nullptr, b == 1)) return 0;
            break;
        case 0x521:
            g_env->browser(thing());
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

u32 __cdecl Panel7cc3b0_DismissAnswer(u8 *staff) {
    g_env->dismiss(staff);
    return 1;
}
}
