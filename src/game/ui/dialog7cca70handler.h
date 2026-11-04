// The handler of the dialog [0x7cca70]. Reimplementation of 0x4c1a10; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, 0x5b6070 by address; the bench's recorders: its own bench has them).
struct Dialog7cca70HandlerEnv {
    u32(__fastcall *focus)(u8 *window, void *, u32 a);                                   // Window_SetFocus
    void(__fastcall *leaveSession)(u8 *session);                                         // 0x5b6070
    void(__fastcall *clearScenes)(u8 *object);                                           // Class702da0_Clear
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                           // Window_SendMessage
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                           // Window7059f0_NthItemData
    void(__cdecl *open)(const u8 *record, u8 *parent, u32 callback, u32 a, u32 b);       // Dialog7cc984_Open
    s32(__fastcall *handler)(u8 *window, void *, u32 message, u32 a, u32 b);             // Window_CallDefault
};
void Dialog7cca70Handler_SetEnv(const Dialog7cca70HandlerEnv *env);   // nullptr: the game's

extern "C" {

// 0x4c1a10 (cdecl (window, message, a, b)). 0x14: [0x7cca70] = 0, [[0xf82884] + 4] given the focus (0). 0x100 with a -2, or 0x1000b with a 0x1b (Escape): 0x5b6070 on the session, Class702da0_Clear of [[0xf85614] + 0x14],
// [0x7cca70] sent (4, 0, 0), the function [0x7cca8c] called when set, answer 0 (no default). 0x400 (child a there, b not -1): Dialog7cc984_Open(row b's data, window, 0, 0x4c0830, 0). Last, but for the one above, Window_CallDefault; its answer.
s32 __cdecl Dialog7cca70_Handler(u8 *window, u32 message, u32 a, u32 b);
}
