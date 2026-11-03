// The thing panels [0x7cc258] and [0x7cc298]: their handlers. Reimplementations of 0x4b0b30 and 0x4b2150; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct ThingPanelHandlerEnv {
    s32(__fastcall *close)(u8 *object);                                        // WindowAt10_Message4
    void(__fastcall *showStats258)(u8 *self);                                  // Panel7cc258_ShowStats
    void(__fastcall *showStats298)(u8 *self);                                  // Panel7cc298_ShowStats
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                 // ThingPanel_Handler_48d200
    u32(__fastcall *setTimer)(u8 *window, void *, u32 id, u32 interval);       // Window_SetTimer
    u32(__fastcall *startTimer)(u8 *window, void *, u32 id);                   // Window_StartTimer
    void(__cdecl *cancelAdvisor)(u32 id);                                      // Game_CancelAdvisor
    void(__cdecl *swing)();                                                    // Sign_Swing
    void(__cdecl *open7ca150)(s32 tab, u32 selected);                          // Dialog7ca150_Open
    void(__fastcall *stepGroup)(u8 *owner);                                    // Things_StepGroup
    void(__fastcall *stepBack)(u8 *owner);                                     // Things_StepGroupBack
    void(__fastcall *restart)(u8 *panel, void *, u32 restart);                 // ThingPanel_RestartOrClose_48ccf0
    void(__fastcall *goTo)(u8 *panel);                                         // ThingGoto_Start
    void(__fastcall *moveMode)(u8 *self);                                      // Panel7cc218_EnterMoveMode
    s32(__cdecl *forEach)(u32 mask, u32 callback);                             // Things_ForEachSelected
};
void ThingPanelHandlers_SetEnv(const ThingPanelHandlerEnv *env);

extern "C" {

// 0x4b0b30 (cdecl (window, message, a, b)). Message 5 closes the panel (answers 1). 0x10 with the timer 0x80080: the stats shown. 0x14: the advisor's 0xd1 cancelled. 0x15: the
// timer 0x80080 of 0xfa0 set and started. 0x1e: the sign swings. 0x800: the sliders 0xc06b ([0x7cc27c], and the child 0xc07f of [0x7cc268] slot 7 with 1 << b), 0xc085
// ([0x7cc284]) and 0xc086 ([0x7cc280]), each then the panel's slot 5. 0x100: -2 and -1 close and 0xc067 opens the objects dialog on the shown thing, 0xc068 goes to the thing,
// 0xc080 enters move mode, 0xc081 applies the panel to the selected things (0x200) and calls slot 5: these four answer 0 without the base handler; 0xc069 / 0xc06a step the
// group, 0xc082 restarts (b is 1) or closes and shows the stats. All else, and after those, the base handler ThingPanel_Handler_48d200.
s32 __cdecl Panel7cc258_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b2150 (cdecl (window, message, a, b)): as 0x4b0b30 for the panel [0x7cc298] and its stats: the advisor's 0xd0; 0x800: 0xa08e ([0x7cc2bc]), 0xa08f ([0x7cc2c4]), 0xa090 ([0x7cc2c0]), each then
// slot 5; 0x100: 0xa089 opens the objects dialog's tab 2, 0xa08b goes to the thing, 0xa0a8 enters move mode, 0xa0a9 applies (0x100, 0x4b20f0); 0xa08c / 0xa08d step, 0xa08a restarts or closes.
s32 __cdecl Panel7cc298_Handler(u8 *window, u32 message, u32 a, u32 b);
}
