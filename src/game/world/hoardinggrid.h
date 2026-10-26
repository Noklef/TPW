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

// 0x5328e0 (cdecl; row, col, pass, fill; name unconfirmed): a flood fill of the copy at 0x81d7c8 (0x42 x 0x42 chars, rows of 0x43) that
// 0x5321d0 makes: from (row, col), while both are 0 to 0x41 and the char is ' ' or '@', it becomes the fill's low byte and the fill
// recurses up, down and left, then steps right. pass is handed on unused. A fill of ' ' or '@' recurses until the stack runs out (sic).
void __cdecl HoardingGrid_Flood(s32 row, s32 col, u32 pass, u32 fill);

// 0x532a50 (cdecl; x1, y1, x2, y2; name unconfirmed): whether some cell of the rectangle from (x1, y1) to (x2, y2) (inclusive, by rows; none
// when x1 > x2 or y1 > y2) that is on the map (Map_InBounds) would be '#' in HoardingGrid_Fill's grid: its 0x44-byte cell (the world's
// +0x294 + id * 0x44) names (MapHandle_Entry28) an entry (its parent's for a 0xc / 0x11 one) not of type 0x19, and lacks flag 0x40. 1 or 0.
u32 __cdecl HoardingGrid_AreaBlocked(s32 x1, s32 y1, s32 x2, s32 y2);

// 0x531a50 (cdecl; x1, y1, x2, y2, charge; name unconfirmed): whether hoardings can enclose the rectangle the two corners span. Its four
// side lists (0x81eb10 top, 0x81ec18 bottom, 0x81eb98 left, 0x81ea78 right: 0x80 bytes of 0xff, counts at 0x81b484 / 0x81edec /
// 0x81aeec / 0x81b73c, and 0x81af4c the cost, all cleared first) get a byte for every second cell of each side (0 a flag 0x40 cell that
// is not type 0x19). 0 when either corner is off the map or a side spans 2 or less, when a side meets one of the two points at 0x7639f0
// / 0x7639f8, when two cells in a row on a side are type 7 / 0x1e (not 2), when both ends are hoarded (type 0x19) and the straight
// hoarding counts (corrected at the corners) all come to 0, when the area is all of one kind (flag 0x40 cells not hoarded, or the rest)
// and no side has two hoarded cells, or when the grid copy (0x81eeb0 into 0x81d7c8, a border of ' ', the area '#', flooded from the
// outside with '#') leaves a ' ' or '@' inside. With charge, each flag 0x40 cell inside the border adds the price at 0x78560c to the cost;
// when the game is not free (0x785a2c) and the cost exceeds the money (Source_Field0c of the world's table entry) 0x816d5c is set and the
// answer is 0. Else 1.
s32 __cdecl HoardingGrid_CheckArea(s32 x1, s32 y1, s32 x2, s32 y2, u8 charge);
}
