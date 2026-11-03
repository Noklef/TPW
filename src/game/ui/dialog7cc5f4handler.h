// The dialog [0x7cc5f4]'s handler. Reimplementation of 0x4baf10 and 0x4bb030; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x5b5d10 unported, agent B's, by address; the bench's recorders).
struct Dialog7cc5f4HandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                     // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);                     // Window_PostMessage
    u32(__cdecl *restoreFocus)();                                                  // Focus_Restore
    void(__cdecl *enableKeys)();                                                   // KeyMaps_EnableAllAndClearFlag
    void(__cdecl *resetKeys)();                                                    // KeyMaps_ResetAllAndSetFlag
    u8 *(__cdecl *online)();                                                       // Online_Instance
    void(__fastcall *apply)(u8 *online, void *, u8 *a, u8 *b);                     // 0x5b5d10
    u8 *(__fastcall *find)(u8 *window, void *, u32 id);                            // Window_FindChild
    u32(__fastcall *focus)(u8 *window, void *, u32 a);                             // Window_SetFocus
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                     // Dialog_BaseHandler
};
void Dialog7cc5f4_SetHandlerEnv(const Dialog7cc5f4HandlerEnv *env);

extern "C" {

// 0x4baf10 (cdecl (window, message, a, b)): 0 unless said. 0x14: [0x7cc5f4] = 0, the window [[0xf82884]+4] sent 6 (1, 0), Focus_Restore, KeyMaps_EnableAllAndClearFlag. 0x15: that window sent 6 (0, 0), KeyMaps_ResetAllAndSetFlag. 0x100: a -1: 0x5b5d10 on Online_Instance (0x7cc590, 0x7cc548). a -2 and 0x804: the dialog sent 4 (0, 0), the callback [0x7cc5fc] when set, and 1.
// 0x802: a 0x49 the child 0x4b focused (Window_SetFocus (0)); else the window posted 0x100 (-1, 0). 0x803: a 0x49 the child 0x4b focused, else the child 0x49.
s32 __cdecl Dialog7cc5f4_Message(u8 *window, u32 message, u32 a, u32 b);
// 0x4bb030 (cdecl (window, message, a, b)): 0 when Dialog7cc5f4_Message answers not 0, else Dialog_BaseHandler's answer.
s32 __cdecl Dialog7cc5f4_Handler(u8 *window, u32 message, u32 a, u32 b);
}
