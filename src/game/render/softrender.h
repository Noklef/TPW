// The software renderer's buffer clear: reimplementation of 0x573ca0.
//
// The software renderer (0x18 bytes at [0x879dac], made by 0x573960 / 0x573a30; "Initializing
// Software renderer") gets at 0x573b90 a buffer of width * height dwords (0x8bcbd0, 0x8bcbd4) at
// +8, its size in bytes at +0xc, and hands it to the rasteriser's globals (0xfbe024).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x573ca0 (thiscall): the buffer's +0xc bytes zeroed (before each scene, 0x57b705); returns 1.
u32 __fastcall SoftRenderer_ClearBuffer(u8 *renderer);
}
