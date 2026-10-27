#include "dialog7ca150.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/language.h"
#include "../world/advisor.h"
#include "../world/ridesweep.h"
#include "buttontext.h"
#include "focus.h"
#include "hoverlabel.h"
#include "listrows.h"
#include "radiogroup.h"
#include "window.h"
#include "window7059f0.h"
#include "windowreg.h"
#include "windowscript.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

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
    {0x00750ab8, 0, 5, 0x00750d5c, {0x80, 0x2000}, {0x00493e90, 0x00493eb0}, 0x52, {0x69, 0x6a, 0x6b, 0x6c, 0x6d}},
    {0x00750ab8, 1, 5, 0x00750d60, {0x200, 0}, {0x00493f30, 0}, 0x57, {0x69, 0x248, 0x249, 0x24a, 0x24b}},
    {0x00750ba0, 2, 6, 0x00750d64, {0x100, 0}, {0x00494070, 0}, 0x5c, {0x69, 0x248, 0x6b, 0x249, 0x24a, 0x24b}},
    {0x00750ca0, 3, 2, 0x00750d68, {0x400, 0}, {0x004941e0, 0}, 0x62, {0x69, 0x24c}},
};

void show_tab(u32 selected, const Tab &tab) {
    u8 *const page = Window_FindChild(global<u8 *>(0x007ca150), nullptr, 0x12c4b8);
    u8 *child = Window_FindChild(page, nullptr, 400);
    if (!child) child = Window_FindChild(page, nullptr, 400);   // sic: the same search three times
    if (!child) child = Window_FindChild(page, nullptr, 400);
    if (child) Window_SendMessage(child, 4, 0, 0);
    u8 *const list =
        WindowScript_Build(Window_FindChild(global<u8 *>(0x007ca150), nullptr, 0x12c4b8), nullptr, reinterpret_cast<u8 *>((uintptr_t)tab.script), 0);
    global<u32>(0x007ca154) = tab.state;
    Window_SetHandler(list, nullptr, 0x004955e0);
    Window7059f0_BuildRows(list, nullptr, &List7ca158_NewRow, &List7ca158_InitRow);   // (0x4944d0, 0x494690: only called, not stored)
    for (s32 i = 0; i < tab.columns; ++i) Window7059f0_SetColumnBottom(list, nullptr, (u32)i, i ? 1u : 0u);
    Window7059f0_SetHighlight(list, nullptr, reinterpret_cast<u8 *>((uintptr_t)GuiWindows_FindByName(reinterpret_cast<const char *>(0x0074fa84))));
    Window7059f0_SortBy(list, nullptr, global<s32>(tab.sort), 0);
    global<u8 *>(0x007ca14c) = list;
    for (s32 i = 0; i < 2 && tab.masks[i]; ++i) Things_ForEachSelected(tab.masks[i], reinterpret_cast<ObjectCallback>((uintptr_t)tab.callbacks[i]));
    Window7059f0_RefreshRows(list);
    if (selected) {
        const s32 position = Window7059f0_FindItemData(list, nullptr, selected);
        if (position >= 0) Window7059f0_ScrollTo(list, nullptr, position);
    }
    for (s32 i = 0; i < tab.columns; ++i) {   // (the header buttons: the list's children 0x10 on)
        u8 *const button = Window_FindChild(list, nullptr, 0x10 + (u32)i);
        const u32 font = global<u32>(0x00750d58);
        u16 *const text = Language_WideText(tab.first_text + (u32)i);
        ButtonText_Init(button, (u32)(uintptr_t)text, font, 0xff, 0xef, 0, tab.values[i]);
    }
}

}  // namespace

extern "C" {

void __cdecl Dialog7ca150_ShowTab0(u32 selected) { show_tab(selected, kTabs[0]); }

void __cdecl Dialog7ca150_ShowTab1(u32 selected) { show_tab(selected, kTabs[1]); }

void __cdecl Dialog7ca150_ShowTab2(u32 selected) { show_tab(selected, kTabs[2]); }

void __cdecl Dialog7ca150_ShowTab3(u32 selected) { show_tab(selected, kTabs[3]); }

void __cdecl Dialog7ca150_Open(s32 tab, u32 selected) {
    if (tab == -1) tab = global<s32>(0x007ca154);
    Window7c24c8_Close();
    Global7cb28c_Select(2, 5);
    global<u8 *>(0x007ca150) = WindowScript_Build(global<u8 *>(0x007cb2ac), nullptr, reinterpret_cast<u8 *>((uintptr_t)0x007508e0), 0x00495290);
    Window7c24c8_Set(global<u8 *>(0x007ca150));
    u8 *const group = Window_FindChild(Window_FindChild(global<u8 *>(0x007ca150), nullptr, 0x12c4b8), nullptr, 0x12c4b9);
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
        HoverLabel_Attach(global<u8 *>(0x007ca150), 0x12c4be, text);
    }
    Window_SetHandler(Window_FindChild(global<u8 *>(0x007ca150), nullptr, 0x12c4b8), nullptr, 0x00495360);
    Advisor_PostId(0xbf);
}
}  // extern "C"
