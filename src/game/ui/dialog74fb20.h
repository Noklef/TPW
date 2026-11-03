// The dialog of the script 0x74fb20 in the overlay [0x7cb2b4] (up [0x7c24cc]; the key Escape or a click after 2 seconds closes it): reimplementation of 0x48adb0 and 0x48a740. The names are invented.
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default, the unported 0x42a190 raw; the bench's recorders: their own benches have them).
struct Dialog74fb20Env {
    u32(__cdecl *toggled)();                                                // Global7cb2e8_Get
    u32(__cdecl *toggle)();                                                 // Overlay7cb2b4_Toggle
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);   // WindowScript_Build
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);      // Window_GetRect
    void(__fastcall *setWords)(u8 *window, void *, u32 a, u32 b);           // Window_SetWordsE4
    u16(__fastcall *runScript)(u8 *window, void *, u8 *script, s32 interval);   // Window_RunScript
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                   // Window_SetFocus
    u32(__cdecl *clock)();                                                  // GuiClock_Now
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);              // Window_SendMessage
    void(__cdecl *stopFollow)();                                            // 0x42a190, agent B's, unported
    void(__cdecl *enableKeys)();                                            // KeyMaps_EnableAll
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Dialog74fb20_SetEnv(const Dialog74fb20Env *env);

extern "C" {

// 0x48adb0 (cdecl; called from 0x4815fe): up [0x7c24cc] cleared, the overlay toggled when it is not (Global7cb2e8_Get, Overlay7cb2b4_Toggle), the dialog built in it with this handler, moved to half its rectangle's
// size (Window_GetRect inner, Window_SetWordsE4), the script 0x74fc38 run (0x23), the overlay focused.
void __cdecl Dialog74fb20_Open();

// 0x48a740 (cdecl (window, message, a, b)): message 0x14 clears up, 0x15 keeps the clock in [0x7c2508], both then the default. 0x10006 with a 1, the byte [0x78d911], more than 0x7d0 since that clock and up: closed
// (the window sent 4, 0x42a190, the overlay toggled, the dialog built anew in it and placed as above, the keys enabled before it is placed, the byte [0x7c24d0] cleared), 0. 0x1000b: with up, the key 0x1b and none of
// b's bits 0xff0000 closed the same way (the keys enabled after it is placed); 0 either way. Anything else the default (Window_CallDefault).
u32 __cdecl Dialog74fb20_Handler(u8 *window, u32 message, u32 a, u32 b);
}
