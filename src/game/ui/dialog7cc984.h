// The dialog [0x7cc984]: its handler. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4c0880 (cdecl; window, message, a, b; name invented, unconfirmed). Closing means: the window's parent (Fn_65d55a) the desktop [0xfaa5d8]: the owner's window ([0xf82884] + 4) sent (6, 1, 0); the game part +0x14 cleared; [0x7cc984] sent (4, 0, 0).
// Message 0x1000b with a = 0x1b, and 0x100 with a = -2: closed so, the callback [0x7cca48] (when set) called, 0 returned. Message 0x100 with a = 0xe592e or 0xe592f, the window closes (the parent the owner's window: the part cleared and
// [0x7cc984] sent (4, 0, 0); else the parent sent (4, 0, 0)): 0xe592e first asks the session (0x5b86a0: 0x7cc988, the callback 0x4c07d0, 0) after it; 0xe592f first has the session unpublish the park [0x7cc990] and after it the owner's window
// sent (6, 1, 0), then the part +8's slot 9 answering: its slot 10; else the part +0xc's slot 0x11. Both 0. Message 0x14: [0x7cc984] = 0; the parent, there and its byte +4 clear: the focus on it (0); else Focus_Restore. Else and after 0x14 Window_CallDefault.
s32 __cdecl Dialog7cc984_Handler(u8 *window, u32 message, u32 a, u32 b);
}

// The handler's calls (the bench's recorders; their own benches have them). Fn_65d55a and Window_CallDefault run. 0x5b86a0 is unported (agent B's range).
struct Dialog7cc984Env {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);               // Window_SendMessage
    void(__fastcall *clear)(u8 *object);                                     // Class702da0_Clear
    u8 *(__cdecl *online)();                                                 // Online_Instance
    u32(__fastcall *unpublish)(u8 *session, void *, u32 park);               // Online_UnpublishPark
    u32(__fastcall *request)(u8 *session, void *, u32 a, u32 callback, u32 c);   // 0x5b86a0
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                    // Window_SetFocus
    u32(__cdecl *restoreFocus)();                                            // Focus_Restore
};
void Dialog7cc984_SetEnv(const Dialog7cc984Env *env);

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8 *, gDialog7cc984_7cc984, 0x007cc984, 0x4);
