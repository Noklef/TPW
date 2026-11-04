// The front-end frame, the object [0xf82884] points to (class 0x7029f0: its constructor, destructor and ticks in src/game/core/vdtor and
// src/game/core/vlist): reimplementation of its close, 0x5d5cf0 (thiscall).
#pragma once

#include "../../../re/types.h"

struct FrameEnv {
    u32(__cdecl *showChild)();                                                      // DialogF82884_ShowChild
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);    // Window_CallDefault
};

void Frame_SetEnv(const FrameEnv *env);

extern "C" {

// 0x5d5dd0 (thiscall (window, message, wparam, lparam), pops 16; name invented, unconfirmed): 0x11 with wparam 1: DialogF82884_ShowChild. 0x14: +4 = 0 with a frame at [0xf82884]. 0x22: +0x1c = 0.
// 0x10003: +0x20 and +0x24 = the high and low words of wparam. 0x10004 and 0x10005: wparam 0, 1 or 2 clears and sets bits 1, 2, 4 of +0x1c. Then with a frame the list at +0x18 (as below) gets slot 5 of each
// object with (message, wparam, lparam). Returns Window_CallDefault's answer.
s32 __fastcall Frame_HandleMessage(u8 *frame, void *, u8 *window, u32 message, u32 wparam, u32 lparam);

// 0x5d5cf0 (thiscall; called from 0x54e7c0 on [0xf82884]; name invented, unconfirmed): over the list whose sentinel is at +0x18 (the first
// node at its +4; nodes {object, next}, the next read first) each object's slot 4 is called; then the objects [0xf83784], [0xf83778],
// [0xf83774], [0xf85614] and +0x10 each deleted (slot 0 with 1) unless 0, and cleared; Sound_ReleaseLobbySfx; the message 4 (0, 0) sent to
// the window +4 (Window_SendMessage). Returns the dword at +0x14.
u32 __fastcall Class7029f0_Close(u8 *frame);
}
