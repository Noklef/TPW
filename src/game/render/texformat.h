// The renderer settings' texture format helpers: reimplementation of 0x5839a0 and 0x5839e0.
//
// The renderer settings are the object at 0x8bd550 (its +0 the renderer flags; +4 the current
// texture format, read all over as 0x8bd554). 0x5839f0 fills its tables, 0x1000 bytes per format
// from +0x88. With the software driver (0x8bd524 == 4) the pixel format at +0x68, laid out as a
// DDPIXELFORMAT, is made 32-bit ARGB (0x5727c3).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5839a0 (thiscall): the DDPIXELFORMAT at +0x68 = size 0x20, flags 0x41 (RGB with alpha), no
// FourCC, 32 bits, masks 0xff0000, 0xff00, 0xff and 0xff000000.
void __fastcall TexFormat_SetArgb32(u8 *settings);

// 0x5839e0 (thiscall): the current format's table, +0x88 + 0x1000 * (+4).
u8 *__fastcall TexFormat_Table(u8 *settings);
}
