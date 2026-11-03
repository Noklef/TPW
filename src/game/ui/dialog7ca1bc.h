// The dialog whose window is at 0x7ca1bc (built at start-up from the template 0x7514c0, its handler 0x497a70: on 0x100 its buttons
// 0x1b59-0x1b5b open other panels; released by Global7ca1bc_Release). Reimplementation of 0x497b20 and 0x497a70. The names are invented.
#pragma once

#include "../../../re/types.h"

// Dialog7ca1bc_Handler's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca1bcHandlerEnv {
    void(__cdecl *open7cc444)();                                       // Dialog7cc444_Create
    void(__cdecl *open7cb25c)();                                       // Dialog7cb25c_Open
    void(__cdecl *open7cc2ec)();                                       // Dialog7cc2ec_Open
    void(__cdecl *resetChild21)(u32 clear27);                          // Dialog7cb2b8_ResetChild21
    s32(__fastcall *fallback)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Dialog7ca1bc_SetHandlerEnv(const Dialog7ca1bcHandlerEnv *env);

extern "C" {

// 0x497a70 (cdecl (window, message, a, b); name invented, unconfirmed): 0x14: [0x7ca1bc] = 0. 0x100: a 0x1b59, 0x1b5a, 0x1b5b: Dialog7cc444_Create, Dialog7cb25c_Open, Dialog7cc2ec_Open, then Dialog7cb2b8_ResetChild21 (1). Then the default.
s32 __cdecl Dialog7ca1bc_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x497b20 (cdecl; name invented): the dialog built on the GUI root [0xfaa5d8] from the template 0x7514c0 with the handler 0x497a70
// (WindowScript_Build) as [0x7ca1bc], taken out of the root's children (Window_Unlink), its dword +0xa0 index 0 set to 0
// (Window_SetDwordA0) and hidden (Window_SetShown(0)); none of it checked.
void __cdecl Dialog7ca1bc_Create();
}
