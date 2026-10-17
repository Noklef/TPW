// The coarse grid of the hoardings' placing: reimplementation of 0x532960.
//
// While the hoardings are placed (0x81b4cc set, the area's width at 0x81d740 and depth at
// 0x81b1b0 set by 0x52f050) a character grid at 0x81eeb0 (rows of 0x43 bytes, one char per 2 x 2
// cells) marks the map: ' ' free, '#' blocked, '@' a type 0x19 cell. 0x5321d0 copies it on.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x532960 (cdecl; name unconfirmed): for each cell (x, y) of the width x depth area, the char
// at 0x81eeb0 + (x >> 1) * 0x43 + (y >> 1) is set to ' ', then, for a cell on the map, to '@'
// when the world's 0x28-byte cell of its id (its parent's for a 0xc / 0x11 cell) is of type
// 0x19, else to '#' when the 0x44-byte cell named by that cell's first word lacks flag 0x40 (the
// char is never '@' at that point, as it was just set to ' ').
void __cdecl HoardingGrid_Fill();
}
