#include "panel7ca228handler.h"

#include <cstdint>

#include "../core/vcall.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/ridesweep.h"
#include "../world/world.h"
#include "dialog7ca098.h"
#include "dialog7ca150.h"
#include "dialogs.h"
#include "panel7cc218.h"
#include "signrect.h"
#include "thinggoto.h"
#include "window.h"
#include "windowtimers.h"
#include "../core/staticobjects.h"

extern "C" u32 __cdecl ThingPanel_Handler_48d200(u8 *window, u32 message, u32 a, u32 b);

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Panel() { return reinterpret_cast<u8 *>((uintptr_t)&gStatic_7ca228); }

u32 __fastcall Word18Game(u8 *, void *, u32 id) { return ObjectControls_GetWord18(id); }

const Panel7ca228HandlerEnv kGameEnv = {
    &WindowAt10_Message4,
    &Things_StepGroupBack,
    &Things_StepGroup,
    &ThingGoto_Start,
    &Panel7cc218_EnterMoveMode,
    &Object_GetDef,
    &Object_ScrapRefund,
    &GameTable7cfb90_Get1da720_519510,
    &Word18Game,
    reinterpret_cast<u32(__fastcall *)(const u8 *, void *, u32)>(&Object_FollowCamera),
    &Window_SetTimer,
    &Window_StartTimer,
    &Sign_Swing,
    &Game_CancelAdvisor,
    &Dialog7ca150_Open,
    &ThingPanel_Handler_48d200,
};
const Panel7ca228HandlerEnv *g_env = &kGameEnv;

void Refresh() {
    const Panel7ca228HandlerEnv *const E = g_env;
    u8 *const thing = gPanel7cc218_7c2658;
    E->getDef(thing);
    u8 *const window = global<u8 *>((u32)(uintptr_t)&gStatic_7ca228 + 0x14);
    if (!window) return;
    u8 *const word = Window_FindChild(window, nullptr, 0x1586);
    if (word) {
        const u32 id = 0x80000u | at<u16>(thing, 0xe);  // (the word in the low half of a's stack slot)
        const u32 value = E->word18(reinterpret_cast<u8 *>((uintptr_t)E->analyser(gWorldPtr)), nullptr, id);
        vcall<void>(word, 0x1c, value);
    }
    u8 *const refund = Window_FindChild(global<u8 *>((u32)(uintptr_t)&gStatic_7ca228 + 0x14), nullptr, 0x1584);
    vcall<void>(refund, 0x1c, E->scrapRefund(thing));
}

}  // namespace

extern "C" {

void Panel7ca228Handler_SetEnv(const Panel7ca228HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

void __fastcall Panel7ca228_Follow(u8 *panel) {
    g_env->followCamera(gPanel7cc218_7c2658, nullptr, 0);
    vcall<void>(panel, 0x2c);
}

u32 __cdecl Panel7ca228_Handler(u8 *window, u32 message, u32 a, u32 b) {
    const Panel7ca228HandlerEnv *const E = g_env;
    switch (message) {
    case 5:
        E->closePanel(Panel());
        return 1;
    case 0x10:
        if (a == 0x80080) Refresh();
        break;
    case 0x14:
        E->cancelAdvisor(0x12a);
        break;
    case 0x15:
        if (static_cast<u16>(E->setTimer(window, nullptr, 0x80080, 0xfa0))) E->startTimer(window, nullptr, 0x80080);
        break;
    case 0x1e:
        E->swing();
        break;
    case 0x100:
        switch (a) {
        case 0xffffffffu:
            E->closePanel(Panel());
            return 0;
        case 0x157b:
            E->gotoStart(Panel());
            return 0;
        case 0x157c:
            E->stepBack(Panel());
            break;
        case 0x157d:
            E->step(Panel());
            break;
        case 0x1580:
            E->openTab(3, 0);
            return 0;
        case 0x1581:
            Panel7ca228_Follow(Panel());
            return 0;
        case 0x1582:
            E->enterMoveMode(Panel());
            return 0;
        }
        break;
    }
    return E->fallback(window, message, a, b);
}
}
