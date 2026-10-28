// The front-end frame, the object [0xf82884] points to (class 0x7029f0: its constructor, destructor and ticks in src/game/core/vdtor and
// src/game/core/vlist): reimplementation of its close, 0x5d5cf0 (thiscall).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d5cf0 (thiscall; called from 0x54e7c0 on [0xf82884]; name invented, unconfirmed): over the list whose sentinel is at +0x18 (the first
// node at its +4; nodes {object, next}, the next read first) each object's slot 4 is called; then the objects [0xf83784], [0xf83778],
// [0xf83774], [0xf85614] and +0x10 each deleted (slot 0 with 1) unless 0, and cleared; Sound_ReleaseLobbySfx; the message 4 (0, 0) sent to
// the window +4 (Window_SendMessage). Returns the dword at +0x14.
u32 __fastcall Class7029f0_Close(u8 *frame);
}
