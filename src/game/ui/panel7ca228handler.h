// The handler of the thing panel at 0x7ca228 (the panel of a ride-like thing, Panel7ca228_Show) and its follow button.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the handler calls (the benches point these at recorders): the ports.
struct Panel7ca228HandlerEnv {
    s32(__fastcall *closePanel)(u8 *panel);                                      // WindowAt10_Message4 0x48cf70
    void(__fastcall *stepBack)(u8 *panel);                                       // Things_StepGroupBack 0x48cbe0
    void(__fastcall *step)(u8 *panel);                                           // Things_StepGroup 0x48caf0
    void(__fastcall *gotoStart)(u8 *panel);                                      // ThingGoto_Start 0x48cd10
    void(__fastcall *enterMoveMode)(u8 *panel);                                  // Panel7cc218_EnterMoveMode 0x48cfa0
    u8 *(__fastcall *getDef)(const u8 *object);                                  // Object_GetDef 0x4dd4e0
    u32(__fastcall *scrapRefund)(const u8 *object);                              // Object_ScrapRefund 0x4e2400
    u32(__fastcall *analyser)(u8 *world);                                        // GameTable7cfb90_Get1da720_519510
    u32(__fastcall *word18)(u8 *controls, void *, u32 id);                       // ObjectControls_GetWord18 0x4ca8c0
    u32(__fastcall *followCamera)(const u8 *object, void *, u32 arg);            // Object_FollowCamera 0x4e15b0
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);         // Window_SetTimer 0x65ef90
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);                     // Window_StartTimer 0x65f024
    void(__cdecl *swing)();                                                      // Sign_Swing 0x4864f0
    void(__cdecl *cancelAdvisor)(u32 id);                                        // Game_CancelAdvisor 0x486b40
    void(__cdecl *openTab)(s32 tab, u32 selected);                               // Dialog7ca150_Open 0x495aa0
    u32(__cdecl *fallback)(u8 *window, u32 message, u32 a, u32 b);               // ThingPanel_Handler_48d200
};
// The handler's calls go to env (nullptr: the ports).
void Panel7ca228Handler_SetEnv(const Panel7ca228HandlerEnv *env);

// 0x499b90 (thiscall; name invented, unconfirmed): the camera follows the thing [0x7c2658] (Object_FollowCamera (0)), then the panel's slot 11.
void __fastcall Panel7ca228_Follow(u8 *panel);

// 0x499bb0 (cdecl (window, message, a, b); name invented, unconfirmed). 5: the panel closed, 1. 0x10 with the timer 0x80080: the thing's
// Object_GetDef, and with a window at 0x7ca23c its child 0x1586 (if any) shown the controls' word for the thing's id and its child 0x1584
// (not checked) the scrap refund, both by slot 7. 0x14: Game_CancelAdvisor (0x12a). 0x15: the timer 0x80080 set to 4000, started if
// that answers (a word). 0x1e: Sign_Swing. 0x100 with a: -1 the panel closed, 0; 0x157b ThingGoto_Start, 0; 0x157c Things_StepGroupBack;
// 0x157d Things_StepGroup; 0x1580 Dialog7ca150_Open (3, 0), 0; 0x1581 Panel7ca228_Follow, 0; 0x1582 Panel7cc218_EnterMoveMode, 0. All else
// and those without a return: ThingPanel_Handler_48d200's answer.
u32 __cdecl Panel7ca228_Handler(u8 *window, u32 message, u32 a, u32 b);
}
