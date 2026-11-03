// The game panel object 0x7cc328's window handler (name invented, unconfirmed): reimplementation of 0x4b4fc0.
#pragma once

#include "../../../re/types.h"

struct Panel7cc328HandlerEnv {
    s32(__fastcall *close)(u8 *object);                                                                        // WindowAt10_Message4
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                                                 // Window7059f0_NthItemData
    void(__fastcall *setTexts)(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32 unused);  // Window7059f0_SetItemTexts
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);                                       // Window_SetTimer
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);                                                   // Window_StartTimer
    void(__cdecl *cancel)(u32 id);                                                                             // Game_CancelAdvisor
    void(__fastcall *gotoThing)(u8 *panel);                                                                    // ThingGoto_Start
    void(__fastcall *step)(u8 *owner);                                                                         // Things_StepGroup
    void(__fastcall *stepBack)(u8 *owner);                                                                     // Things_StepGroupBack
    void(__cdecl *jobs)(s32 tab, u32 selected);                                                                // Dialog7ca150_Open
    void(__cdecl *staffList)(u32 tab, u32 selected);                                                           // Dialog7ca168_Open
    void(__fastcall *moveMode)(u8 *self);                                                                      // Panel7cc218_EnterMoveMode
    void(__fastcall *showStaff)(u8 *panel, void *, const u8 *staff);                                           // Panel7cc328_ShowStaff
    void(__cdecl *kill)(s32 slot);                                                                             // SpriteScript_Kill
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                                                 // ThingPanel_Handler_48d200
    void(__fastcall *show)(u8 *self, void *, u8 *thing);                                                       // Panel7cc3b0_Show
};
void Panel7cc328Handler_SetEnv(const Panel7cc328HandlerEnv *env);

extern "C" {

// 0x4b4fc0 (cdecl (window, message, a, b)): 5: WindowAt10_Message4 on the panel, 1. 0x10 with a 0x80080: each row of the list 0x1e7b of [0x7cc338] from the last: the cell of the mask 2 (Window7059f0_SetItemTexts, refreshing at row 0) the item's +0x1fc's low byte (truncated) * 1024 / 100. 0x14: Game_CancelAdvisor(0x12c).
// 0x15: the timer 0x80080 set to 2000 and, if that answers (a word) other than 0, started. 0x100: a -1: WindowAt10_Message4; 0x1e7a: ThingGoto_Start; 0x1e83: Dialog7ca168_Open([0x7cc358], 0); 0x1e87: Panel7cc218_EnterMoveMode; 0x1e88: Dialog7ca150_Open(3, [0x7c2658]): 0 for these; 0x1e85, 0x1e86: Things_StepGroupBack,
// Things_StepGroup on the panel. 0x400: the child a's item b's data (a child, b not -1) shown (Panel7cc3b0_Show on 0x7cc3b0); 0. 0x401: with a child a: b not -1, the item b's data Panel7cc328_ShowStaff; b -1, with a script [0x7cc34c]: SpriteScript_Kill and [0x7cc34c] = 0, [0x7cc350] = [0x7cc354] = -1. 0x406: [0x756f40] = b.
// All else: ThingPanel_Handler_48d200's answer.
s32 __cdecl Panel7cc328_Handler(u8 *window, u32 message, u32 a, u32 b);
}
