// The handler of the child 0x25 of the window [0x7cb2b8]. Reimplementation of 0x4a0840; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x5f0b40 unported, agent B's, by address; the bench's recorders).
struct Dialog7cb2b8TabsEnv {
    void(__cdecl *reopen)(s32 group);                                               // DialogTabs_Reopen
    void(__cdecl *post)();                                                          // Dialog7ca1e0_Post_498bb0
    void(__cdecl *cancel)();                                                        // Dialog7ca1e0_Cancel_498bd0
    void(__fastcall *other)(u8 *object);                                            // 0x5f0b40
    void(__cdecl *research)();                                                      // ResearchPanel_Open
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);    // Window_CallDefault
};
void Dialog7cb2b8_SetTabsEnv(const Dialog7cb2b8TabsEnv *env);

extern "C" {

// 0x4a0840 (cdecl (window, message, a, b)): message 0x100 with the control a: 0x26, 0x28, 0x2a DialogTabs_Reopen (1, 2, 3); 0x27 with b 1 Dialog7ca1e0_Post, with any other Dialog7ca1e0_Cancel; 0x29 0x5f0b40 on 0xf86db8; 0x2b ResearchPanel_Open. The answer always Window_CallDefault's.
s32 __cdecl Dialog7cb2b8_TabsHandler(u8 *window, u32 message, u32 a, u32 b);
}
