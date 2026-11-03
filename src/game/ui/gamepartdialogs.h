// The handlers of the two small dialogs that drive the game object's parts: [0x7cc47c] and [0x7cc4b4]. Reimplementations of 0x4b86a0 and 0x4b8b70; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them). Window_CallDefault runs.
struct GamePartDialogEnv {
    void(__cdecl *open)(u32 value);                             // Dialog7cca70_Open
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);   // Class702da0_Push
    void(__cdecl *setEffect)(u8 *window, u32 on);               // Dialog7cc4b4_SetEffect
    void(__cdecl *update)();                                    // Dialog7cc4b4_Update
};
void GamePartDialogs_SetEnv(const GamePartDialogEnv *env);

extern "C" {

// 0x4b86a0 (cdecl (window, message, a, b); name invented, unconfirmed). Message 0x14: [0x7cc47c] cleared. 0x100, on the game [0xf85614] read for each call: 0x49d5 its +8's slot 0xb and
// Dialog7cca70_Open (0x4b8510); 0x49d6 +8's slot 8, +4's slot 7 and +0x14's Class702da0_Push (0x258, 1); 0x49d7 +8's slot 8 and +0xc's slot 7: these three answer 0. 0x49d8, 0x49db and
// 0x49dc +8's slot 0x10, 0xf and 0xe. All else, and after those, Window_CallDefault.
s32 __cdecl Dialog7cc47c_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4b8b70 (cdecl (window, message, a, b); name invented, unconfirmed). Message 0x11: a not 0 Dialog7cc4b4_Update, a 0 the effect off. 0x14: [0x7cc4b4] cleared and the effect off
// (Dialog7cc4b4_SetEffect (0, 0)). 0x100, on the game [0xf85614] read for each call: 0x1e0e9 its +4's slot 8 and +0xc's slot 7, answering 0. 0x1e0e8, 0x1e0f0 and 0x1e0f1 +4's slot 0x10, 0xf and 0xe.
// All else, and after those, Window_CallDefault.
s32 __cdecl Dialog7cc4b4_Handler(u8 *window, u32 message, u32 a, u32 b);
}
