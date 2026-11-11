// The park statistics dialog [0x7ca2e8]. Reimplementation of 0x49ac60 and its handler 0x49a0b0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca2e8HandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);     // Window_SendMessage
    void(__cdecl *cancelAdvisor)(u32 id);                          // Game_CancelAdvisor
    u16 *(__cdecl *wideText)(u32 id);                              // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);           // HoverLabel_Attach
    s32(__cdecl *keyHandler)(u8 *window, u32 message, u32 a, u32 b);   // Dialog_KeyHandler
    void(__cdecl *open7cb25c)();                                   // Dialog7cb25c_Open
    void(__cdecl *open7cc2ec)();                                   // Dialog7cc2ec_Open
    void(__cdecl *open7ca210)();                                   // Dialog7ca210_Open
};
void Dialog7ca2e8_SetHandlerEnv(const Dialog7ca2e8HandlerEnv *env);

// Dialog7ca2e8_Open's calls.
struct Dialog7ca2e8OpenEnv {
    s32(__cdecl *atExit)(void(__cdecl *f)());                      // 0x67a7d0
    u16(__fastcall *select)(u8 *group, void *, u32 id);            // RadioGroup_Select
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);   // Window705a50_SetState
};
void Dialog7ca2e8_SetOpenEnv(const Dialog7ca2e8OpenEnv *env);

extern "C" {

// 0x49a0b0 (cdecl (window, message, a, b)): 5: [0x7ca2e8] sent 4 (0, 0), answer 1. 0x14: [0x7ca2e8] = 0, Game_CancelAdvisor (0xc5), then Dialog_KeyHandler. 0x15: the hover label on the child 0x12271 (the text 0xa1), then Dialog_KeyHandler. 0x100: a -2 or -1:
// [0x7ca2e8] sent 4, answer 0; a 0x1227f, 0x12280, 0x12281: Dialog7cb25c_Open, Dialog7cc2ec_Open, Dialog7ca210_Open, answer 0. All else Dialog_KeyHandler.
s32 __cdecl Dialog7ca2e8_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x49ac60 (cdecl): nothing when open. Window7c24c8_Close, Global7cb28c_Select (3, 7), the script 0x751ca8 built on [0x7cb2ac] with the handler 0x49a0b0 (none built: nothing more) as [0x7ca2e8], Window7c24c8_Set. Its child 0x12272, when there is one, the graph: a RendererPart_Ctor part
// (0xd8 bytes) and the handler 0x49a2e0; its child 0x12273, when there is one, the radio group with the buttons 0x12275, 0x12276, 0x12274 (the texts 0x16a-0x16c) and [0x7ca2ec] 0, 1, 2 selecting them; the buttons 0x12277, 0x1227b, 0x12279, 0x12278, 0x1227c, 0x1227d, 0x1227a, 0x1227e (the
// texts 0xa2-0xa9), each set (Window705a50_SetState (1, 1)) when its bit of [0x751fa0], from bit 0, is set. The mode setting (the guard 0xfb34c3 bit 0, Setting_ReadMode, atexit 0x5d1750) 2: the dialog's child 0x1227f's slot 5 given 0. Finally, graph or not, Advisor_PostId (0xc5). Each button: a
// TextPart_Ctor part, the bytes 1, 1, font 0xb, black, no text, slot 9 the text. No child but those two is checked.
void __cdecl Dialog7ca2e8_Open();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gDialog7ca2e8_751fa0, 0x00751fa0, 0x4);
GAME_DATA(u8, gDialog7ca2e8_fb34c3, 0x00fb34c3, 0x1);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x2f8], gDialog7ca2e8_Open_751ca8, 0x00751ca8, 0x2f8);   // a window script (or a text / table before one): to the next named address
