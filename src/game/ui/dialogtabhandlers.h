// The handlers of three tabbed dialogs: their tab buttons, close and text. Reimplementation of 0x498c60, 0x4b24b0 and 0x4b7a70; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct DialogTabHandlersEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);      // Window_SendMessage
    s32(__cdecl *keys)(u8 *window, u32 message, u32 a, u32 b);      // Dialog_KeyHandler
    void(__cdecl *cancelAdvisor)(u32 id);                           // Game_CancelAdvisor
    u32(__fastcall *table)(u8 *game);                               // GameTable7cfb90_Get1da726_5195d0
    void(__fastcall *set118)(u8 *object, void *, u32 value);        // Obj_Set118
    void(__fastcall *openPark)(u8 *game, void *, u32 open, u32 gateOnly);  // Game_OpenPark
    void(__cdecl *open7ca2e8)();                                    // Dialog7ca2e8_Open
    void(__cdecl *open7cb25c)();                                    // Dialog7cb25c_Open
    void(__cdecl *open7cc2ec)();                                    // Dialog7cc2ec_Open
    void(__cdecl *open7ca210)();                                    // Dialog7ca210_Open
    void(__cdecl *open7cb398)();                                    // Dialog7cb398_Create
    u32(__fastcall *money)(u8 *game);                               // GameTable7cfb90_Get1da718_519450
};
void DialogTabHandlers_SetEnv(const DialogTabHandlersEnv *env);

extern "C" {

// All cdecl (window, message, a, b); anything not told here ends in Dialog_KeyHandler, whose answer is the answer.
// 0x498c60: 5: [0x7ca210] sent 4 (0, 0), answer 1. 0x14: [0x7ca210] = 0, the game's table entry (GameTable7cfb90_Get1da726_5195d0) told Obj_Set118 ([0x7ca1e8]), Game_CancelAdvisor (0x12f). 0x800: [0x7ca1e8] = b. 0x100: a -2 or -1 [0x7ca210] sent 4, answer 0; 0x4f3ab Dialog7ca2e8_Open, 0x4f3ac Dialog7cb25c_Open,
// 0x4f3ad Dialog7cc2ec_Open, answer 0; 0x4f3b1 Game_OpenPark (b not 1, 0).
s32 __cdecl Dialog7ca210_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b24b0: 5: [0x7cc2ec] sent 4 (0, 0), answer 1. 0x14: [0x7cc2ec] = 0. 0x100: a -1 [0x7cc2ec] sent 4, answer 0; 0x2db Dialog7ca210_Open, 0x2dc Dialog7ca2e8_Open, 0x2dd Dialog7cb25c_Open, answer 0. 0x800: a 0x2e7, 0x2ec, 0x2ea, 0x2eb, 0x2ed: the budget's dword +0xc, +0x10, +0x14, +0x18, +0x1c (GameTable7cfb90_Get1da718_519450)
// b, or 0 when b is negative, its byte +0x20 1 when above 0; then with the child 0x2de: its child 0x2df's slot 7 the five's sum, 0x2e4's slot 7 [0x7cc2d4] - the sum - [0x7cc2e4].
s32 __cdecl Dialog7cc2ec_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b7a70: 5: [0x7cc444] sent 4 (0, 0), answer 1. 0x14: [0x7cc444] = 0. 0x100: a -2 [0x7cc444] sent 4, answer 0; 0xadb4 Dialog7cb398_Create, 0xadb5 Dialog7ca2e8_Open, answer 0.
s32 __cdecl Dialog7cc444_Handler(u8 *window, u32 message, u32 a, u32 b);
}
