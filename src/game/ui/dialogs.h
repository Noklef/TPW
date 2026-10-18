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
}
