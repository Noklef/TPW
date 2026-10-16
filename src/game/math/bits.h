// Bit counting for colour masks: reimplementation of 0x5911d0 and 0x5911f0.
//
// The texture formats (0x583a6f) and the landscape reader (0x563ecb) turn each of a pixel
// format's red, green, blue and alpha masks into a width and a shift with these.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5911d0: how many bits are set.
u32 __cdecl Bits_Count(u32 mask);

// 0x5911f0: how many clear bits are below the lowest set one (0 for 0).
u32 __cdecl Bits_Shift(u32 mask);
}
