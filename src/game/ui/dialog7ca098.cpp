#include "dialog7ca098.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../world/advisor.h"
#include "../world/camera.h"
#include "../world/challenges.h"
#include "../world/object.h"
#include "../world/world.h"
#include "dialog7ca210.h"
#include "dialog7ca308.h"
#include "dialog7cb25c.h"
#include "dialog7cb398.h"
#include "dialog7cc1f8.h"
#include "dialog7cc2ec.h"
#include "dialog7cc444.h"
#include "dialogs.h"
#include "levelmeter.h"
#include "msgboxbutton.h"
#include "window.h"
#include "windowreg.h"
#include "dialog7cb2b8build.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
const Dialog7ca098Env kGameEnv = {Camera_CentreOnThing, Camera_CentreOnCell, Dialog7cc1f8_Open, Dialog7ca308_Open, Dialog7cb25c_Open, ResearchPanel_Open, Dialog7cb398_Create, Dialog7cc2ec_Open, Dialog7cc444_Create, Dialog7ca210_Open, Object_SetFlag64,
                                  Challenges_AcceptOffer, Challenges_DeclineOffer, Window_NotifyImpl100, MsgBox_SetButton, Window_SetObject98, GuiWindows_FindByName, Window_SetDepth, Window_RunScript, Window_StopScript, Advisor_Cancel, Window_SendMessage,
                                  Window_PostMessage, Dialog7cb2b8_ResetChild21, Window_CallDefault};
const Dialog7ca098Env *g_env = &kGameEnv;

u8 *box() { return gDialogs_7ca098; }
u8 *button(u32 id) { return Window_FindChild(box(), nullptr, id); }
void depthUp(u32 id) {
    u8 *const child = button(id);
    g_env->setDepth(child, nullptr, (u16)(*reinterpret_cast<u16 *>(child + 0xe0) + 1));
}

void refresh(u8 *window) {
    if (u8 *const text = button(0x10d32)) {
        g_env->notify(text, nullptr, Window_DwordA0(window, nullptr, 1));
        u8 *child = button(0x10d30);
        g_env->setButton(child, Window_DwordA0(box(), nullptr, 3));
        child = button(0x10d2f);
        g_env->setButton(child, Window_DwordA0(box(), nullptr, 4));
        g_env->setObject(button(0x10d2e), nullptr, nullptr);
        if (Window_DwordA0(box(), nullptr, 4) == 0x1b) {
            vcall<void>(button(0x10d2e), 0x14, 0u);
        } else {
            u8 *const erase = reinterpret_cast<u8 *>((uintptr_t)g_env->findByName(gDialog7ca098_750614));
            g_env->setObject(button(0x10d2e), nullptr, erase);
            vcall<void>(button(0x10d2e), 0x14, 1u);
        }
    }
    depthUp(0x10d30);
    depthUp(0x10d2f);
    depthUp(0x10d2e);
    reinterpret_cast<u32 &>(gDialog7ca098_7ca090) = reinterpret_cast<u32 &>(gDialog7ca098_7ca094);
    g_env->runScript(gDialog7ca098_7ca094, nullptr, gDialog7ca098_7502b8, 100);
}

// The button's action (its kind the dword kindAt), what the answer asks for.
void clicked(u8 *window, u32 kindAt) {
    u8 *const open = gDialog7ca098_7ca090;
    const u32 a = Window_DwordA0(window, nullptr, 2), b = Window_DwordA0(window, nullptr, 5);
    const u32 answer = Dialog7ca098_RunAction(Window_DwordA0(window, nullptr, kindAt), a, b);
    if (answer == 2) g_env->send(gDialog7cb2b8build_7cb2c4, 0x100002, (u32)(uintptr_t)open, 0);
    else if (answer != 1) return;
    if (reinterpret_cast<u32 &>(gDialog7ca098_7ca090)) g_env->resetChild21(0);
}
}  // namespace

void Dialog7ca098_SetEnv(const Dialog7ca098Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Game_CancelAdvisor(u32 id) { g_env->cancelAdvisor(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da71c_5194d0(gWorldPtr)), nullptr, id); }

u32 __cdecl Dialog7ca098_RunAction(u32 kind, u32 a, u32 b) {
    switch (kind) {
    case 3: g_env->centreOnThing(a); return 0;
    case 4: g_env->centreOnCell(reinterpret_cast<const u16 *>(&b)); return 0;
    case 9: g_env->openList(0, b); return 2;
    case 0xa: g_env->openList(1, b); return 2;
    case 0xb: g_env->openList(2, b); return 2;
    case 0xc: g_env->openList(3, b); return 2;
    case 0xd: case 0xe: case 0xf: case 0x10: case 0x11: g_env->openTab(kind - 0xd); return 2;
    case 0x12: g_env->open49fb30(); return 2;
    case 0x13: g_env->open4aa480(); return 2;
    case 0x14: g_env->open4a52a0(); return 2;
    case 0x15: g_env->open4b2750(); return 2;
    case 0x16: g_env->open4b7b00(); return 2;
    case 0x17: g_env->setFlag64(global<u8 *>(0x007cfb90 + (a & 0xffff) * 20), nullptr, 1); return 1;
    case 0x18: g_env->open498d80(); return 2;
    case 0x1a: g_env->accept(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da736_519610(gWorldPtr))); return 2;
    case 0x1b: g_env->decline(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da736_519610(gWorldPtr))); return 2;
    default: return 0;
    }
}

s32 __cdecl Dialog7ca098_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x1d) {
        if (a == 1) reinterpret_cast<u32 &>(gDialog7ca098_7ca094) = 0;
    } else if (message == 0x1001d9) {
        refresh(window);
    } else if (message == 0x1001d8) {
        if (u8 *const open = gDialog7ca098_7ca090) g_env->stopScript(open);
        Game_CancelAdvisor(0x11c);
        reinterpret_cast<u32 &>(gDialog7ca098_7ca090) = 0;
    } else if (message == 0x100) {
        if (a == 0x10d30) {
            clicked(window, 3);
        } else if (a == 0x10d2f) {
            clicked(window, 4);
        } else if (a == 0x10d2e) {
            if (u8 *const open = gDialog7ca098_7ca090) {
                vcall<void>(open, 0x14, 0u);
                g_env->post(gDialog7cb2b8build_7cb2c4, 0x100002, reinterpret_cast<u32 &>(gDialog7ca098_7ca090), 0);
                if (reinterpret_cast<u32 &>(gDialog7ca098_7ca090)) g_env->resetChild21(0);
            }
        }
    }
    return g_env->callDefault(window, nullptr, message, a, b);
}
}
