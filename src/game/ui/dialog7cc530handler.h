// The dial-up dialog [0x7cc530]'s handler (name invented, unconfirmed): reimplementation of 0x4ba530.
#pragma once

#include "../../../re/types.h"

struct Dialog7cc530HandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                  // Window_SendMessage
    u32(__cdecl *restore)();                                                    // Focus_Restore
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                  // Dialog_BaseHandler
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                  // Window7059f0_NthItemData
    void(__cdecl *create)(u32 owner);                                           // Dialog7cc5f4_Create
    void(__cdecl *createFrom)(u32 value, u8 *data);                             // Dialog7cc5f4_CreateFrom
    void(__cdecl *fill)(u8 shown);                                              // DialupList_Fill
};
void Dialog7cc530Handler_SetEnv(const Dialog7cc530HandlerEnv *env);

extern "C" {

// 0x4ba530 (cdecl (window, message, a, b)): message 0x14: [0x7cc530] = 0, the window [[0xf82884] + 4] sent (6, 1, 0), Focus_Restore; 0x15: that window sent (6, 0, 0). Message 0x100, a -2: the dialog sent (4, 0, 0), the function [0x7cc52c] called if set; a -1: with the dialog's radio
// child 0xd's +0x138 other than 0xf, the dialog sent (4, 0, 0) and Dialog7cc5f4_Create([0x7cc52c]); with 0xf, the selected item's data of the list child 0xc (Window7059f0_NthItemData of its +0x14c) if not negative: the connection's properties (import 0x6fd5bc on 0x7c24c0) and, with
// their first dword set, [0x7cc53c] = 1, the name +0x212 in 0x7cc4f0 ("%S"), then the same send and create; with it 0, the send and Dialog7cc5f4_CreateFrom(data, properties). Those answer 0. Message 0x101 with b 1: DialupList_Fill(a != 0xe). Everything else, as after 0x14, 0x15 and 0x101: Dialog_BaseHandler.
s32 __cdecl Dialog7cc530_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[20], gDialog7cc530handler_7cc4f0, 0x007cc4f0, 0x14);
GAME_DATA(u8, gDialog7cc530handler_7cc53c, 0x007cc53c, 0x1);
