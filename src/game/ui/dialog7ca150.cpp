#include "dialog7ca150.h"
#include "dialog7ca098.h"
#include "dialog7ca13c.h"
#include "dialog7ca168.h"
#include "dialog7cb398.h"
#include "dialogbase.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../world/advisor.h"
#include "../world/camera.h"
#include "../world/ridesweep.h"
#include "../world/things.h"
#include "buttontext.h"
#include "focus.h"
#include "hoverlabel.h"
#include "listrows.h"
#include "radiogroup.h"
#include "thingpanel.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"
#include "panel7b9940.h"
#include "dialog7cc188.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7ca150Env kGameEnv = {{&Dialog7ca150_ShowTab0, &Dialog7ca150_ShowTab1, &Dialog7ca150_ShowTab2, &Dialog7ca150_ShowTab3}, &Language_WideText, &HoverLabel_Attach, &ThingPanel_Show};
const Dialog7ca150Env *g_env = &kGameEnv;

// a tab of the dialog: its script, the columns (their header texts from first_text on, the buttons' values), the sort dword, the things'
// groups and their callbacks
struct Tab {
    u32 script;
    u32 state;
    s32 columns;
    u32 sort;
    u32 masks[2];
    u32 callbacks[2];
    u32 first_text;
    u32 values[6];
};

const Tab kTabs[4] = {
    {reinterpret_cast<uintptr_t>(&gDialog7ca150_750ab8), 0, 5, reinterpret_cast<uintptr_t>(&gDialog7ca150_750d5c[0]), {0x80, 0x2000}, {0x00493e90, 0x00493eb0}, 0x52, {0x69, 0x6a, 0x6b, 0x6c, 0x6d}},
    {reinterpret_cast<uintptr_t>(&gDialog7ca150_750ab8), 1, 5, reinterpret_cast<uintptr_t>(&gDialog7ca150_750d5c[1]), {0x200, 0}, {0x00493f30, 0}, 0x57, {0x69, 0x248, 0x249, 0x24a, 0x24b}},
    {0x00750ba0, 2, 6, reinterpret_cast<uintptr_t>(&gDialog7ca150_750d5c[2]), {0x100, 0}, {0x00494070, 0}, 0x5c, {0x69, 0x248, 0x6b, 0x249, 0x24a, 0x24b}},
    {reinterpret_cast<uintptr_t>(&gDialog7ca150_750ca0), 3, 2, reinterpret_cast<uintptr_t>(&gDialog7ca150_750d5c[3]), {0x400, 0}, {0x004941e0, 0}, 0x62, {0x69, 0x24c}},
};

void show_tab(u32 selected, const Tab &tab) {
    u8 *const page = Window_FindChild(gDialog7ca150_7ca150, nullptr, 0x12c4b8);
    u8 *child = Window_FindChild(page, nullptr, 400);
    if (!child) child = Window_FindChild(page, nullptr, 400);   // sic: the same search three times
    if (!child) child = Window_FindChild(page, nullptr, 400);
    if (child) Window_SendMessage(child, 4, 0, 0);
    u8 *const list =
        WindowScript_Build(Window_FindChild(gDialog7ca150_7ca150, nullptr, 0x12c4b8), nullptr, reinterpret_cast<u8 *>((uintptr_t)tab.script), 0);
    gDialog7ca150_7ca154 = tab.state;
    Window_SetHandler(list, nullptr, 0x004955e0);
    Window7059f0_BuildRows(list, nullptr, &List7ca158_NewRow, &List7ca158_InitRow);   // (0x4944d0, 0x494690: only called, not stored)
    for (s32 i = 0; i < tab.columns; ++i) Window7059f0_SetColumnBottom(list, nullptr, (u32)i, i ? 1u : 0u);
    Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>((u32)(uintptr_t)&gText_Dialog7cc188_74fa84))));
    Window7059f0_SortBy(list, nullptr, global<s32>(tab.sort), 0);
    gDialog7ca150_7ca14c = list;
    for (s32 i = 0; i < 2 && tab.masks[i]; ++i) Things_ForEachSelected(tab.masks[i], reinterpret_cast<ObjectCallback>((uintptr_t)tab.callbacks[i]));
    Window7059f0_RefreshRows(list);
    if (selected) {
        const s32 position = Window7059f0_FindItemData(list, nullptr, selected);
        if (position >= 0) Window7059f0_ScrollTo(list, nullptr, position);
    }
    for (s32 i = 0; i < tab.columns; ++i) {   // (the header buttons: the list's children 0x10 on)
        u8 *const button = Window_FindChild(list, nullptr, 0x10 + (u32)i);
        const u32 font = gDialog7ca150_750d58;
        u16 *const text = Language_WideText(tab.first_text + (u32)i);
        ButtonText_Init(button, (u32)(uintptr_t)text, font, 0xff, 0xef, 0, tab.values[i]);
    }
}

}  // namespace

void Dialog7ca150_SetEnv(const Dialog7ca150Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

u32 __cdecl Dialog7ca150_Handler(u8 *window, u32 message, u32 a, u32 b) {
    switch (message) {
    case 0x101: {
        static const u8 kTabOfRadio[] = {0, 2, 1, 3};
        const u32 radio = a - 0x12c4ba;
        if (b == 1 && radio <= 3) {
            const u32 tab = kTabOfRadio[radio];
            g_env->showTab[tab](0);
            const u32 text = (u32)(uintptr_t)g_env->text(0x4e + tab);
            g_env->attach(gDialog7ca150_7ca150, 0x12c4be, text);
        }
        break;
    }
    case 0x400: {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {
            u32 slot = (u32)(uintptr_t)window;   // (0x400)
            Window7059f0_NthItemData(child, nullptr, (s32)b, &slot);
            g_env->thingPanel(reinterpret_cast<u8 *>((uintptr_t)slot));
        }
        return 0;
    }
    case 0x402: {
        u8 *const child = Window_FindChild(window, nullptr, a);
        if (child && b != 0xffffffff) {   // (0x402)
            u32 slot = (u32)(uintptr_t)window;   // (0x402)
            Window7059f0_NthItemData(child, nullptr, (s32)b, &slot);
            u16 handle;
            Thing_GetHandle(reinterpret_cast<const u8 *>((uintptr_t)slot), nullptr, &handle);
            Camera_CentreOnThing(handle);
            Window_SendMessage(gDialog7ca150_7ca150, 4, 0, 0);
            return 0;
        }
        break;
    }
    case 0x406: {
        const u32 tab = gDialog7ca150_7ca154;
        if (tab <= 3) gDialog7ca150_750d5c[tab] = b;
        break;
    }
    }
    return (u32)Window_CallDefault(window, nullptr, message, a, b);
}

void __cdecl Dialog7ca150_ShowTab0(u32 selected) { show_tab(selected, kTabs[0]); }

void __cdecl Dialog7ca150_ShowTab1(u32 selected) { show_tab(selected, kTabs[1]); }

void __cdecl Dialog7ca150_ShowTab2(u32 selected) { show_tab(selected, kTabs[2]); }

void __cdecl Dialog7ca150_ShowTab3(u32 selected) { show_tab(selected, kTabs[3]); }

void __cdecl Dialog7ca150_Open(s32 tab, u32 selected) {
    if (tab == -1) tab = reinterpret_cast<s32 &>(gDialog7ca150_7ca154);
    Window7c24c8_Close();
    Global7cb28c_Select(2, 5);
    gDialog7ca150_7ca150 = WindowScript_Build(gPanel7b9940_7cb2ac, nullptr, gDialog7ca150_Open_7508e0, 0x00495290);
    Window7c24c8_Set(gDialog7ca150_7ca150);
    u8 *const group = Window_FindChild(Window_FindChild(gDialog7ca150_7ca150, nullptr, 0x12c4b8), nullptr, 0x12c4b9);
    switch ((u32)tab) {
    case 0:
        RadioGroup_Select(group, nullptr, 0x12c4ba);
        Dialog7ca150_ShowTab0(selected);
        break;
    case 1:
        RadioGroup_Select(group, nullptr, 0x12c4bc);
        Dialog7ca150_ShowTab1(selected);
        break;
    case 2:
        RadioGroup_Select(group, nullptr, 0x12c4bb);
        Dialog7ca150_ShowTab2(selected);
        break;
    case 3:
        RadioGroup_Select(group, nullptr, 0x12c4bd);
        Dialog7ca150_ShowTab3(selected);
        break;
    }
    if ((u32)tab <= 3) {
        const u32 text = (u32)(uintptr_t)Language_WideText(0x4e + (u32)tab);
        HoverLabel_Attach(gDialog7ca150_7ca150, 0x12c4be, text);
    }
    Window_SetHandler(Window_FindChild(gDialog7ca150_7ca150, nullptr, 0x12c4b8), nullptr, 0x00495360);
    Advisor_PostId(0xbf);
}
}  // extern "C"

namespace {
const Dialog7ca150PageEnv kGamePageEnv = {Window_SendMessage, Dialog7cb398_Create, Dialog7ca168_Open, Dialog7ca13c_Open, Game_CancelAdvisor, Dialog_KeyHandler};
const Dialog7ca150PageEnv *g_page = &kGamePageEnv;

u8 *&pageDialog() { return gDialog7ca150_7ca150; }
}  // namespace

void Dialog7ca150_SetPageEnv(const Dialog7ca150PageEnv *env) { g_page = env ? env : &kGamePageEnv; }

extern "C" s32 __cdecl Dialog7ca150_PageHandler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 5) {
        g_page->send(pageDialog(), 4, 0, 0);
        return 1;
    }
    if (message == 0x14) {
        pageDialog() = nullptr;
        g_page->cancel(0xbf);
    } else if (message == 0x100) {
        switch (a) {
        case 0xfffffffeu:
            g_page->send(pageDialog(), 4, 0, 0);
            return 0;
        case 0x12c4bf:
            g_page->openStats();
            return 0;
        case 0x12c4c0:
            g_page->openStaff(0xffffffffu, 0);
            return 0;
        case 0x12c4c1:
            g_page->openGuests(0);
            return 0;
        }
    }
    return g_page->keys(window, message, a, b);
}
