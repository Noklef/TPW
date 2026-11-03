// The thing panels [0x7cc258], [0x7cc298], 0x7ca178, 0x7cc408 and 0x7cc3e0: their handlers. Reimplementations of 0x4b0b30, 0x4b2150, 0x497640, 0x4b7720 and 0x4b67f0; the names are invented, unconfirmed.
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
    void(__fastcall *showStats178)(u8 *self);                                  // Panel7ca178_ShowStats
    void(__fastcall *setFlag)(u8 *panel, void *, u32 flag);                    // ThingPanel_SetFlag64_48ce50
    void(__cdecl *copyScript)();                                               // Global7c24d8_CopyScript
    void(__fastcall *showVisitor)(u8 *self);                                   // Panel7cc408_ShowVisitor
    void(__cdecl *openBrowser)(u8 *thing);                                     // ThingBrowser_Open
    void(__cdecl *open7ca13c)(u32 selected);                                   // Dialog7ca13c_Open
    u8 *(__fastcall *def)(const u8 *object);                                   // Object_GetDef
    u32(__fastcall *table)(u8 *game);                                          // GameTable7cfb90_Get1da720_519510
    u32(__stdcall *control)(u32 id);                                           // ObjectControls_GetWord18
    u32(__fastcall *refund)(const u8 *object);                                 // Object_ScrapRefund
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

// 0x497640 (cdecl (window, message, a, b); name invented, unconfirmed): the panel 0x7ca178's. Message 5 closes it (answers 1). 0x10 with the timer 0x80080: the stats shown.
// 0x14: the advisor's 0x12b and 0xd5 cancelled. 0x15: the timer 0x80080 of 0xfa0 set and started. 0x1e: the sign swings. 0x100: -1 closes, 0x15bb4 sets the flag (0 when b is 1,
// else 1), 0x15bb5 goes to the thing, 0x15bc2 opens the objects dialog's tab 3 on the shown thing, 0x15bc3 enters move mode: these answer 0 without the base handler; 0x15bb6 and
// 0x15bb7 step the group back and on, 0x15bc4 restarts (b is 1) or closes and shows the stats. All else, and after those, the base handler ThingPanel_Handler_48d200.
s32 __cdecl Panel7ca178_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b7720 (cdecl (window, message, a, b); name invented, unconfirmed): the panel 0x7cc408's. Message 5 closes it (answers 1). 0x10 with the timer 0x80080: the visitor shown. 0x14: the
// advisor's 0x12e cancelled. 0x15: the timer. 0x1e: Global7c24d8_CopyScript. 0x100: -2 closes, 0x3737 the thing browser, 0x3738 the guests dialog on the shown thing: these answer 0 without
// the base handler; 0x3720 and 0x3721 step the group back and on. All else, and after those, the base handler.
s32 __cdecl Panel7cc408_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b67f0 (cdecl (window, message, a, b); name invented, unconfirmed): the panel 0x7cc3e0's. Message 5 closes it (answers 1). 0x10 with the timer 0x80080: as Panel7ca228_ShowValues on the
// children 0x5210 and 0x520e, with the id's dword holding a's upper half (0x80000 |, read from a's stack slot). 0x14: the advisor's 0x12d cancelled. 0x15: the timer. 0x1e: the sign swings.
// 0x100: -1 closes, 0x5209 goes to the thing, 0x5213 opens the objects dialog (0, 0), 0x5214 enters move mode: these answer 0 without the base handler; 0x520a and 0x520b step the group
// back and on. All else, and after those, the base handler.
s32 __cdecl Panel7cc3e0_Handler(u8 *window, u32 message, u32 a, u32 b);
}
