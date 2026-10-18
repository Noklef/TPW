// The game's dialog windows (each a global window pointer) sent their messages: Window_SendMessage wrappers. Reimplementation of 0x4b87f0,
// 0x4b8810 and 0x4b8830.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b87f0 / 0x4b8810 / 0x4b8830 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc47c] with message 4 (0, 0), message 6
// (1, 0) and message 6 (0, 0); its result returned.
s32 __cdecl Dialog7cc47c_Message4();
s32 __cdecl Dialog7cc47c_Message6On();
s32 __cdecl Dialog7cc47c_Message6Off();

// 0x4b8e80 / 0x4b8ec0 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc4b4] with message 4 (0, 0) and message 6 (0, 0).
s32 __cdecl Dialog7cc4b4_Message4();
s32 __cdecl Dialog7cc4b4_Message6Off();

// 0x4b9c00 / 0x4b9c20 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc4c4] with message 4 (0, 0) and message 6 (1, 0).
s32 __cdecl Dialog7cc4c4_Message4();
s32 __cdecl Dialog7cc4c4_Message6On();

// 0x4c5780 / 0x4c57a0 / 0x4c57c0 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cdb0c] with message 4 (0, 0), message 6
// (1, 0) and message 6 (0, 0).
s32 __cdecl Dialog7cdb0c_Message4();
s32 __cdecl Dialog7cdb0c_Message6On();
s32 __cdecl Dialog7cdb0c_Message6Off();

// 0x4c57e0 / 0x4c5800 (cdecl; names unconfirmed): the same with message 6 (1, 0) / (0, 0) to its child 0x16007 (Window_FindChild; none:
// the send's -1).
s32 __cdecl Dialog7cdb0c_Child16007Message6On();
s32 __cdecl Dialog7cdb0c_Child16007Message6Off();
}
