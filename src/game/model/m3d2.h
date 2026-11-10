// The M3D2 model data loader (the 3D exporter's format): reimplementation of its error box 0x46f100 so far.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46f100 (cdecl; text; name unconfirmed): MessageBoxA(none, text, "M3D2 Exporter - Data Error" (0x74da14), OK / Cancel with the exclamation
// icon (0x31)); 1 unless the answer is Cancel (2).
u32 __cdecl M3D2_DataErrorBox(const char *text);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[28], gText_M3D2_DataErrorBox_74da14, 0x0074da14, 0x1c);
