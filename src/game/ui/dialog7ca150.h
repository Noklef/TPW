// The dialog whose page [0x7ca150] holds the objects' lists (its child 0x12c4b8 the page they are built on; the list [0x7ca14c], its rows
// List7ca158's by the tab [0x7ca154]). Reimplementation of 0x494250, 0x494c70, 0x494ec0, 0x495110 and 0x495aa0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7ca150_Handler's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca150Env {
    void(__cdecl *showTab[4])(u32 selected);
    u16 *(__cdecl *text)(u32 index);                    // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);   // HoverLabel_Attach
    void(__cdecl *thingPanel)(u8 *thing);               // ThingPanel_Show
};
void Dialog7ca150_SetEnv(const Dialog7ca150Env *env);

// Dialog7ca150_PageHandler's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca150PageEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);     // Window_SendMessage
    void(__cdecl *openStats)();                                    // Dialog7cb398_Create
    void(__cdecl *openStaff)(u32 tab, u32 selected);               // Dialog7ca168_Open
    void(__cdecl *openGuests)(u32 selected);                       // Dialog7ca13c_Open
    void(__cdecl *cancel)(u32 id);                                 // Game_CancelAdvisor
    s32(__cdecl *keys)(u8 *window, u32 message, u32 a, u32 b);     // Dialog_KeyHandler
};
void Dialog7ca150_SetPageEnv(const Dialog7ca150PageEnv *env);

extern "C" {

// 0x495360 (cdecl (window, message, a, b): Dialog7ca150_Open's handler of the page's child 0x12c4b8; name invented). 0x101 with b 1 and a a radio 0x12c4ba to 0x12c4bd: the tab 0, 2, 1 or 3
// shown (selected 0) and the hover label 0x12c4be of the page [0x7ca150] given the text 0x4e + the tab. 0x400: the child a (Window_FindChild) when there is one and b is not -1: its item b's
// data (Window7059f0_NthItemData, whose slot is left the window when there is no item b, sic) given to ThingPanel_Show; answer 0 either way. 0x402 the same but the thing's handle
// (Thing_GetHandle) to Camera_CentreOnThing and the page sent 4 (0, 0), answer 0; no child or -1 goes on. 0x406: the tab [0x7ca154] (0-3) has its sort dword (0x750d5c on) b. Otherwise, and
// after these, the default handler's answer (Window_CallDefault).
u32 __cdecl Dialog7ca150_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x494ec0 / 0x494c70 / 0x494250 / 0x495110 (cdecl (selected); the tabs 0-3; names unconfirmed): the page (the child 0x12c4b8 of
// [0x7ca150]) has its child 400 sent 4 (0, 0) when there is one (sic: looked for three times, the same search); then a list is built on the
// page from the tab's script (WindowScript_Build: 0x750ab8 for tabs 0 and 1, 0x750ba0 for 2, 0x750ca0 for 3; no handler), [0x7ca154] the
// tab, the list's handler 0x4955e0, its rows List7ca158_NewRow's initialised by List7ca158_InitRow (Window7059f0_BuildRows), its columns'
// bottoms 0 for the first and 1 for the rest (5 columns, 6 for tab 2, 2 for tab 3), the "hilight" window as its highlight
// (GuiWindows_FindByName, Window7059f0_SetHighlight), sorted by the tab's dword (0x750d5c-0x750d68: Window7059f0_SortBy, not sorting);
// [0x7ca14c] the list; the tab's things added by their callbacks (Things_ForEachSelected: 0x80 0x493e90 then 0x2000 0x493eb0, 0x200
// 0x493f30, 0x100 0x494070, 0x400 0x4941e0); the rows refreshed; a selected data (not 0) found among the items (Window7059f0_FindItemData)
// and scrolled to; then each column's header button (the list's children 0x10 on) given its text (Language_WideText of the tab's ids, the
// font [0x750d58], the colour 0xff, 0xef, 0 and the tab's value: ButtonText_Init). EAX not kept.
void __cdecl Dialog7ca150_ShowTab0(u32 selected);
void __cdecl Dialog7ca150_ShowTab1(u32 selected);
void __cdecl Dialog7ca150_ShowTab2(u32 selected);
void __cdecl Dialog7ca150_ShowTab3(u32 selected);

// 0x495aa0 (cdecl (tab, selected); name unconfirmed): the tab -1 is [0x7ca154]'s; the window [0x7c24c8] closed (Window7c24c8_Close),
// Global7cb28c_Select(2, 5), the page built on [0x7cb2ac] from the script 0x7508e0 with the handler 0x495290 (WindowScript_Build; not
// checked) into [0x7ca150] and made the window [0x7c24c8] (Window7c24c8_Set). Its child 0x12c4b8's child 0x12c4b9 (Window_FindChild,
// neither checked), a radio group, has the tab's button selected (RadioGroup_Select: 0x12c4ba, 0x12c4bc, 0x12c4bb, 0x12c4bd for tabs 0-3)
// and the tab shown with the selected data (Dialog7ca150_ShowTab0-3); then the page's child 0x12c4be gets the hover label of the language
// text 0x4e + the tab (Language_WideText, HoverLabel_Attach). Any other tab (unsigned) none of that. Then the page's child 0x12c4b8 the
// handler 0x495360 (Window_SetHandler) and the advisor told 0xbf (Advisor_PostId). EAX not kept.
void __cdecl Dialog7ca150_Open(s32 tab, u32 selected);

// 0x495290 (cdecl (window, message, a, b); name invented, unconfirmed): the handler of the dialog [0x7ca150]. 5: it is sent 4 (0, 0), answer 1. 0x100: a -2 the same, answer 0; 0x12c4bf opens Dialog7cb398_Create, 0x12c4c0 the staff list (-1, 0),
// 0x12c4c1 the guests' list (0), answer 0. 0x14: [0x7ca150] 0, Game_CancelAdvisor (0xbf). The rest ends in Dialog_KeyHandler, whose answer is returned.
s32 __cdecl Dialog7ca150_PageHandler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[232], gDialog7ca150_750ab8, 0x00750ab8, 0xe8);
GAME_DATA(u8[180], gDialog7ca150_750ca0, 0x00750ca0, 0xb4);
GAME_DATA(u32, gDialog7ca150_750d58, 0x00750d58, 0x4);
GAME_DATA(u32[4], gDialog7ca150_750d5c, 0x00750d5c, 0x10);
GAME_DATA(u8 *, gDialog7ca150_7ca14c, 0x007ca14c, 0x4);
GAME_DATA(u8 *, gDialog7ca150_7ca150, 0x007ca150, 0x4);
GAME_DATA(u32, gDialog7ca150_7ca154, 0x007ca154, 0x4);
