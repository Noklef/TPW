// The handler of the dial-up form (the script 0x7583e8, made by Dialog7cc5f4_CreateFrom). Reimplementation of 0x4b9d00; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, agent B's unported 0x47f020 by address; tests/fncmp/dialog7cc5f4dial.py gives recorders).
struct Dialog7cc5f4DialEnv {
    void(__cdecl *enableKeys)();                                                         // KeyMaps_EnableAllAndClearFlag
    u32(__cdecl *restoreFocus)();                                                        // Focus_Restore
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                           // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);                           // Window_PostMessage
    u32(__fastcall *dwordA0)(u8 *window, void *, u32 index);                             // Window_DwordA0
    u16 *(__cdecl *text)(u32 index);                                                     // Language_WideText
    u8 *(__cdecl *box)(const u8 *record, u32 a, u32 callback, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);                        // Window_SetHandler
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                                // Window_SetFocus
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                           // Dialog_BaseHandler
};
void Dialog7cc5f4Dial_SetEnv(const Dialog7cc5f4DialEnv *env);   // nullptr: the game's

extern "C" {

// 0x4b9d00 (cdecl (window, message, a, b)). 0x14: KeyMaps_EnableAllAndClearFlag, Focus_Restore, [[0xf82884] + 4] sent (6, 1, 0). 0x15: that sent (6, 0, 0). 0x802: a 0x49: child 0x4b focused, else 0x100 (-1, 0) posted to the window. 0x803: child 0x4b (a 0x49) or 0x49 focused. All these and the rest: Dialog_BaseHandler, its answer.
// Answer 0: 0x804: the window sent (4, 0, 0). 0x100 a -2: the same, then the function [0x7cc52c] when set. 0x100 a -1 with the wide texts 0x7cc590 and 0x7cc548 both not empty (else the base): id = Window_DwordA0(0), the window sent (4, 0, 0), [0x7cc50c] = id, [0x7cc508] = [0x7cc504] = -1; the box 0x47f020 (0; ff ff ff; kind 4; id) with 2 and 0x4b9f30 in [0x7cc534], handler 0x4b9f70;
// the import [0x6fd5b4] on 0x7c24c0 (id, 0x4b9f60, 0x7cc590, 0x7cc548, id): answered 0: the box's slot 9 given text 0x1d8.
s32 __cdecl Dialog7cc5f4_DialHandler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, gDialog7cc5f4dial_7cc504, 0x007cc504, 0x4);
GAME_DATA(u32, gDialog7cc5f4dial_7cc50c, 0x007cc50c, 0x4);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x48], gDialog7cc5f4dial_7cc548, 0x007cc548, 0x48);   // the dial fields: to the next named address
