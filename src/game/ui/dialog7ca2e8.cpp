// The dialog [0x7ca2e8]: see dialog7ca2e8.h.
#include "dialog7ca2e8.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../render/renderer.h"
#include "../world/advisor.h"
#include "dialog7ca098.h"
#include "dialog7ca210.h"
#include "dialog7cb25c.h"
#include "dialog7cc2ec.h"
#include "dialogbase.h"
#include "dialogs.h"
#include "focus.h"
#include "hoverlabel.h"
#include "panelparts.h"
#include "radiogroup.h"
#include "window.h"
#include "window705a50.h"
#include "windowscript.h"
#include "dialog74fb20.h"

namespace {

u8 *&dialog() { return *reinterpret_cast<u8 **>((uintptr_t)0x007ca2e8); }

const Dialog7ca2e8HandlerEnv kGameEnv = {&Window_SendMessage, &Game_CancelAdvisor, &Language_WideText, &HoverLabel_Attach, &Dialog_KeyHandler, &Dialog7cb25c_Open, &Dialog7cc2ec_Open, &Dialog7ca210_Open};
const Dialog7ca2e8HandlerEnv *g_env = &kGameEnv;

const Dialog7ca2e8OpenEnv kOpenGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &RadioGroup_Select, &Window705a50_SetState};
const Dialog7ca2e8OpenEnv *g_open = &kOpenGameEnv;

void button(u8 *parent, u32 id, u32 text) {
    u8 *const child = panelparts::withPart(parent, TextPart_Ctor, id);
    Window_NotifyC00(child, nullptr, 1, 1);
    Window_Set4c(child, nullptr, Language_Font(0xb));
    Window_Notify40000(child, nullptr, 0, 0, 0, 0xff);
    Window_NotifyImpl100(child, nullptr, 0);
    vcall<void>(child, 0x24, (u32)(uintptr_t)Language_WideText(text));
}

void dressGraph(u8 *graph) {
    u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0xd8));
    Window_SetImpl(graph, nullptr, memory ? RendererPart_Ctor(memory, nullptr, 0) : nullptr);
    Window_SetHandler(graph, nullptr, 0x0049a2e0);
    if (u8 *const group = Window_FindChild(graph, nullptr, 0x12273)) {
        static const u32 kRadios[3] = {0x12275, 0x12276, 0x12274};
        button(group, 0x12275, 0x16a);
        button(group, 0x12276, 0x16b);
        button(group, 0x12274, 0x16c);
        const u32 selected = *reinterpret_cast<u32 *>((uintptr_t)0x007ca2ec);
        if (selected < 3) g_open->select(group, nullptr, kRadios[selected]);
    }
    static const u32 kButtons[8] = {0x12277, 0x1227b, 0x12279, 0x12278, 0x1227c, 0x1227d, 0x1227a, 0x1227e};
    for (u32 i = 0; i < 8; ++i) button(graph, kButtons[i], 0xa2 + i);
    for (u32 i = 0; i < 8; ++i)
        if (*reinterpret_cast<u8 *>(&gDialog7ca2e8_751fa0) & (1u << i)) g_open->setState(Window_FindChild(graph, nullptr, kButtons[i]), nullptr, 1, 1);
    u8 &guard = gDialog7ca2e8_fb34c3;
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_open->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c == 2) vcall<void>(Window_FindChild(dialog(), nullptr, 0x1227f), 0x14, 0u);
}

}  // namespace

void Dialog7ca2e8_SetHandlerEnv(const Dialog7ca2e8HandlerEnv *env) { g_env = env ? env : &kGameEnv; }
void Dialog7ca2e8_SetOpenEnv(const Dialog7ca2e8OpenEnv *env) { g_open = env ? env : &kOpenGameEnv; }

s32 __cdecl Dialog7ca2e8_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        g_env->send(dialog(), 4, 0, 0);
        return 1;
    }
    if (message == 0x14) {
        dialog() = nullptr;
        g_env->cancelAdvisor(0xc5);
    } else if (message == 0x15) {
        g_env->attach(window, 0x12271, (u32)(uintptr_t)g_env->wideText(0xa1));
    } else if (message == 0x100) {
        if (a == 0xfffffffe || a == 0xffffffff) {
            g_env->send(dialog(), 4, 0, 0);
            return 0;
        }
        if (a == 0x1227f || a == 0x12280 || a == 0x12281) {
            if (a == 0x1227f) g_env->open7cb25c();
            else if (a == 0x12280) g_env->open7cc2ec();
            else g_env->open7ca210();
            return 0;
        }
    }
    return g_env->keyHandler(window, message, a, b);
}

void __cdecl Dialog7ca2e8_Open() {
    if (dialog()) return;
    Window7c24c8_Close();
    Global7cb28c_Select(3, 7);
    u8 *const built = WindowScript_Build(*reinterpret_cast<u8 **>((uintptr_t)0x007cb2ac), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x00751ca8), 0x0049a0b0);
    if (!built) return;
    dialog() = built;
    Window7c24c8_Set(built);
    if (u8 *const graph = Window_FindChild(dialog(), nullptr, 0x12272)) dressGraph(graph);
    Advisor_PostId(0xc5);
}
