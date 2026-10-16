// The software renderer's buffer clear and texture memory: reimplementation of 0x573ca0 and
// 0x573e30.
//
// The software renderer (0x18 bytes at [0x879dac], made by 0x573960 / 0x573a30; "Initializing
// Software renderer") gets at 0x573b90 a buffer of width * height dwords (0x8bcbd0, 0x8bcbd4) at
// +8, its size in bytes at +0xc, and hands it to the rasteriser's globals (0xfbe024).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x573ca0 (thiscall): the buffer's +0xc bytes zeroed (before each scene, 0x57b705); returns 1.
u32 __fastcall SoftRenderer_ClearBuffer(u8 *renderer);

// 0x573e30 (thiscall, a virtual reached through 0x573ef0; the renderer unused): the texture
// memory to use: the option's (0x8c1620, its MB << 20, 0x44ddab) when set, else a quarter of the
// physical memory (0x8bd520, from GlobalMemoryStatus) kept within 8 to 24 MB. The quarter is a
// signed division: 2 GB or more counts as negative and gets 8 MB (sic).
u32 __fastcall SoftRenderer_TextureMemory(u8 *renderer);
}
