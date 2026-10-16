// Palette helpers: reimplementation of 0x5f7360, 0x5f73a0 and 0x47ddc0.
//
// A palette is 256 PALETTEENTRYs (red, green, blue, flags); the game's is at 0xfa71f8.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x5f7360: count RGB triples into RGBX quads (flags 0), in the original's byte order.
void __cdecl Palette_ExpandRgb(u8 *dst, const u8 *src, u32 count);
// 0x5f73a0: the entry nearest (red, green, blue) (each taken & 0xff): the smallest squared
// distance (an exact match at once); ties go to the smallest summed difference, then to the
// smallest blue^2 + 2 * (red^2 + green^2) of the entry, then to the lowest index.
u8 __cdecl Palette_Nearest(const u8 *palette, u32 red, u32 green, u32 blue);
// 0x47ddc0 (thiscall): a colour as blue, green, red, 0xff, then its nearest entry in the game's
// palette (0xfa71f8).
void __fastcall PalColour_Set(u8 *colour, void *, u32 red, u32 green, u32 blue);
}
