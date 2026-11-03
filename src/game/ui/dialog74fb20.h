// The dialogs of the scripts 0x74fb20 and 0x74faf0 in the overlay [0x7cb2b4] (up [0x7c24cc]; the key Escape or a click after 2 seconds closes them): reimplementation of 0x48adb0, 0x48a740, 0x48ac40, 0x48ae70, 0x48a970 and 0x48aff0. The names are invented.
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default, the unported 0x42a190, 0x42ae70, 0x4e15b0, 0x47f020, 0x516c10 and 0x67a7d0 raw; the bench's recorders: their own benches have them).
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
    void(__cdecl *stopFollow)();                                            // Camera_StopFollowing
    void(__cdecl *enableKeys)();                                            // KeyMaps_EnableAll
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
    void(__cdecl *walk)();                                                  // Camera_ToggleWalkMode
    u32(__fastcall *thingOp)(u8 *thing, void *, u32 a);                     // Object_FollowCamera
    void(__cdecl *resetKeys)();                                             // KeyMaps_ResetAll
    u32(__cdecl *restore)();                                                // Focus_Restore
    u8 *(__fastcall *create)(u8 *parent, void *, u32 kind, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);   // Window_CreateChild
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);           // Window_SetHandler
    u8 *(__fastcall *readMode)(u8 *setting);                                // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *f)());                               // 0x67a7d0
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020, agent B's, unported
    void(__fastcall *tableOp)(u8 *object);                                  // 0x516c10, agent A's, unported
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

// 0x48aff0 (cdecl; name invented, unconfirmed): by the bits [0x790ab0]: 2 the walk toggled (0x42ae70); else 4: 0x42a190, and with 0x400 then the walk toggled.
void __cdecl CameraMode_Leave();

// 0x48ac40 (cdecl (thing); name invented, unconfirmed): the shared start: CameraMode_Leave, 0x4e15b0 on the thing (0), the keys reset, the byte [0x7c24d0] set, the overlay toggled when it is (Global7cb2e8_Get), a dialog up [0x7c24c8] sent 5 and forgotten with Focus_Restore, [0x7cb2ac] and [0x7cb2b0] sent 6 (0, 0), a child of [0xfaa5d8]
// (Window_CreateChild (1, 1, 0x7472, 0, 0, 0x7ff, 0x5ff)). Its handler 0x48a740, the dialog 0x74fb20 built in it, placed with the script 0x74fc38, the host focused and up [0x7c24cc].
void __cdecl Dialog74fb20_OpenFor(u8 *thing);

// 0x48ae70 (cdecl (thing); name invented, unconfirmed): the same start; the dialog 0x74faf0 built in the host, its slot 5 (0), placed with the script 0x74fcf8, the host's handler 0x48a970 and the host focused.
void __cdecl Dialog74faf0_OpenFor(u8 *thing);

// 0x48a970 (cdecl (window, message, a, b); name invented, unconfirmed): 0x15 keeps the clock in [0x7c2508], then the default. 0x10006 with a 1, the byte [0x78d911] and more than 0x7d0 since: closed, 0. 0x1000b with the key 0x1b and none of b's bits 0xff0000: closed; 0 either way. Anything else the default.
// Closed: the window sent 4 (0, 0), 0x516c10 on [0x7cf83c], [0x7cb2b0] sent 6 (0, 0), [0x7cb2ac] sent 6 (1, 0), the keys enabled, the byte [0x7c24d0] cleared, Focus_Restore, and with the mode setting (0xfb3b7c, read under the guard 0xfb34c7 & 1) 2 the box of the text 0x1d7.
u32 __cdecl Dialog74faf0_Handler(u8 *window, u32 message, u32 a, u32 b);
}
