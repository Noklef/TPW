// The window handler of the panel at 0x7cc3b0 and its box's answer. Reimplementation of 0x4b5cb0 and 0x4b6070; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct Panel7cc3b0HandlerEnv {
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                              // ThingPanel_Handler_48d200
    s32(__fastcall *message4)(u8 *object);                                                  // WindowAt10_Message4
    void(__cdecl *cancel)(u32 id);                                                          // Game_CancelAdvisor
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);                    // Window_SetTimer
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);                                // Window_StartTimer
    u32(__fastcall *checksCell)(const u8 *record);                                          // AnalyserRecord_ChecksCell
    void(__cdecl *copyScript)();                                                            // Global7c24d8_CopyScript
    void(__fastcall *showStats)(u8 *self);                                                  // Panel7cc3b0_ShowStats
    u32(__fastcall *toIndex)(const u8 *object);                                             // Byte2_ToIndex
    void(__cdecl *openStaff)(u32 tab, u32 selected);                                        // Dialog7ca168_Open
    void(__fastcall *stepGroup)(u8 *owner);                                                 // Things_StepGroup
    void(__fastcall *stepBack)(u8 *owner);                                                  // Things_StepGroupBack
    u32(__fastcall *setPatrol)(u8 *self, void *, u32 on);                                   // Panel7cc3b0_SetPatrol
    u16 *(__cdecl *text)(u32 index);                                                        // Language_WideText
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);  // 0x47f020, agent B's, unported
    void(__fastcall *dismiss)(u8 *staff);                                                   // Staff_Dismiss
    u32(__cdecl *setPicked)(u32 value);                                                     // Global7c2650_Set
    void(__fastcall *pickUp)(u8 *staff);                                                    // Staff_PickUp
    void(__cdecl *browser)(u8 *thing);                                                      // ThingBrowser_Open
};
void Panel7cc3b0_SetHandlerEnv(const Panel7cc3b0HandlerEnv *env);

extern "C" {

// 0x4b5cb0 (cdecl (window, message, a, b)): with T the thing [0x7c2658] and P the panel 0x7cc3b0: message 5 WindowAt10_Message4 of P, answer 1. 0x10 with a 0x80080 Panel7cc3b0_ShowStats. 0x14 the advisors 0xc0 and 0xd4 cancelled. 0x15 the timer 0x80080
// set to 4000 and, when that answers (AX), started. 0x1e with a T: the child 0x50e of [0x7cc3c0] (Window_FindChild; not checked) slot 5 given AnalyserRecord_ChecksCell(T) as 0 or 1, Global7c24d8_CopyScript. 0x100: a -1 WindowAt10_Message4 of P, answer 0;
// 0x50b Dialog7ca168_Open(Byte2_ToIndex(T), T), 0; 0x50c with the byte [0x78d90f] a box (0x47f020: text 0x18d, colour ff ff ff, kind 4, data T; 2, 0, 1, the answer 0x4b6070, 0, 0, 0), without it Staff_Dismiss(T), then P's slot 11, 0;
// 0x50d Things_StepGroupBack(P); 0x50e Global7c2650_Set(T), Staff_PickUp(T), P's slot 11, 0; 0x50f Things_StepGroup(P); 0x510 Panel7cc3b0_SetPatrol(P, b is 1): nonzero answers 0; 0x521 ThingBrowser_Open(T), 0. All else: ThingPanel_Handler_48d200's answer.
s32 __cdecl Panel7cc3b0_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b6070 (cdecl (staff)): the box's answer: Staff_Dismiss(staff), answer 1.
u32 __cdecl Panel7cc3b0_DismissAnswer(u8 *staff);
}
