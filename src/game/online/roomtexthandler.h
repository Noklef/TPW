// The handler of the chat rooms' text windows. Reimplementation of 0x4c56e0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders: their own benches have them).
struct RoomTextHandlerEnv {
    u8 *(__cdecl *online)();                                 // Online_Instance
    u32(__fastcall *dwordA0)(u8 *window, void *, u32 index);   // Window_DwordA0
    void(__fastcall *enter)(u8 *online, void *, s32 room);   // 0x5b7910 (agent B's)
    void(__cdecl *update)();                                 // RoomCounts_Update
    u32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void RoomTextHandler_SetEnv(const RoomTextHandlerEnv *env);

extern "C" {

// 0x4c56e0 (cdecl (window, message, a, b)): message 0x1e the rooms' counts updated; message 0x10006, with the session's four parts +0xe0, +0xf8, +0xfc and +0xf4 each with its +0x28 set, 0x5b7910 on the
// room (the window's first dword +0xa0, less 1). Always ends in the default handler, whose answer it returns.
u32 __cdecl RoomText_Handler(u8 *window, u32 message, u32 a, u32 b);
}
