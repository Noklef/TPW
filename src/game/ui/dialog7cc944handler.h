// The handler of the dialog [0x7cc944], the online game's list. Reimplementation of 0x4bf9f0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, agent B's unported 0x5bcbc0 by address; tests/fncmp/dialog7cc944handler.py gives recorders).
struct Dialog7cc944HandlerEnv {
    u32(__fastcall *focus)(u8 *window, void *, u32 a);                                   // Window_SetFocus
    void(__fastcall *clearScenes)(u8 *object);                                           // Class702da0_Clear
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                           // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);                           // Window_PostMessage
    void(__fastcall *search)(u8 *games, void *, u32 filter, u32 callback, u32 c);        // 0x5bcbc0
    void(__fastcall *clearList)(u8 *window);                                             // Window7059f0_Clear
    u16 *(__cdecl *text)(u32 index);                                                     // Language_WideText
    s32(__fastcall *addItem)(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                           // Window7059f0_NthItemData
    void(__cdecl *open)(const u8 *record, u8 *parent, u32 callback, u32 a, u32 b);       // Dialog7cc984_Open
    s16(__fastcall *stateWord)(const u8 *window);                                        // Window705a50_StateWord
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);                  // Window705a50_SetState
    void(__fastcall *selectAll)(u8 *window);                                             // Window705a20_SelectAll
    s32(__fastcall *handler)(u8 *window, void *, u32 message, u32 a, u32 b);             // Window_CallDefault
};
void Dialog7cc944Handler_SetEnv(const Dialog7cc944HandlerEnv *env);   // nullptr: the game's

extern "C" {

// 0x4bf9f0 (cdecl (window, message, a, b)). 0x14: [0x7cc944] = 0, [[0xf82884] + 4] given the focus (0). 0x1f: child 0xe05a given it. 0x100 with a 0xe059 and b 1: 0x5bcbc0 on Online_Instance's +0xec (0x7cc908, 0x4bf9d0, 0); with [0x7cc944] there its list child 0xe05b cleared and one row added (text 0x134, 0x759158, 0x78a374, 0, 0, 0; -1, -1, 1). With b other: OutputDebugStringA(0x75910c).
// 0x100 with a -2, or 0x1000b with a 0x1b: Class702da0_Clear on [[0xf85614] + 0x14], [0x7cc944] sent (4, 0, 0), the function [0x7cc948] called when set, answer 0 (no default). 0x400 (child a there, b not -1): its row b's data, not 0 or -1: Dialog7cc984_Open(data, window, 0, 0x4c0830, 0); answer 0 (no default).
// 0x802 with child 0xe059's state word 0: 0x100 (0xe059, 1) posted to the window, 0xe059's state set (1, 1), child a's Window705a20_SelectAll. Else, and after these: Window_CallDefault, its answer.
s32 __cdecl Dialog7cc944_Handler(u8 *window, u32 message, u32 a, u32 b);
}
