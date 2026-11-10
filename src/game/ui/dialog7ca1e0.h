// The dialog whose window is at 0x7ca1e0 (built at start-up from the template 0x751720, its handler 0x498ad0: on
// 0x100 its buttons 0x63 / 0x64 act and the sound 0x131 plays; on 0x14 [0x7ca1e0] forgotten). Reimplementation of 0x498b50. The name is
// invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x498b50 (cdecl; name invented): the dialog built on the GUI root [0xfaa5d8] from the template 0x751720 with the handler 0x498ad0
// (WindowScript_Build) as [0x7ca1e0], taken out of the root's children (Window_Unlink), its dword +0xa0 index 0 set to 1
// (Window_SetDwordA0) and hidden (Window_SetShown(0)); none of it checked.
void __cdecl Dialog7ca1e0_Create();

// What the handler calls (the benches point these at recorders): the ports.
struct Dialog7ca1e0Env {
    void(__cdecl *enterMode)();                                                    // MouseMode_Enter6fead0 0x481a10 (the original's ECX 0x7b51f0, unread)
    void(__cdecl *pauseHide)();                                                    // GamePause_Hide 0x4a9380
    void(__cdecl *cancelAdvisor)(u32 id);                                          // Game_CancelAdvisor 0x486b40
    void(__cdecl *resetChild21)(u32 clear27);                                      // Dialog7cb2b8_ResetChild21 0x4a25f0
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault 0x65f66e
};
// The handler's calls go to env (nullptr: the defaults).
void Dialog7ca1e0_SetEnv(const Dialog7ca1e0Env *env);

// 0x498ad0 (cdecl (window, message, a, b); name invented): 0x14: [0x7ca1e0] 0. 0x100: a 0x63 the mouse mode 0x6fead0 entered, 0x64 GamePause_Hide; for any a then Game_CancelAdvisor(0x131)
// and Dialog7cb2b8_ResetChild21(1). Every message then goes to Window_CallDefault, its answer returned.
s32 __cdecl Dialog7ca1e0_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8 *, gDialog7ca1e0_7ca1e0, 0x007ca1e0, 0x4);
