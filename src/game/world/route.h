// A route found between two cells (the search 0x511420; names unconfirmed): reimplementation of its straightening 0x5110d0.
//
// The route: +0 its count of cells (a byte), +2 the cell it starts from, +6 the cells (words, column then row) in order. Cells are tested by the
// walk of linestep (LineStep_Init and its globals) in the steps' mode 0x7cf3c0 (MapCell_StepBlocked).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5110d0 (cdecl; route, first): from the cell before the first-th (the start for 0), each next cell kept is the furthest of the next four (the
// fourth, third, second, then first ahead) that the straight walk to it reaches, else the next; the cells left out are dropped and the count set
// to what is kept (sic: to first itself when that is not under the count). Repeated while it dropped any; 1 when it did, else 0. The walk
// fails at a blocked step or, turning, where a step round either corner is open but the other step of that corner blocked.
u32 __cdecl Route_Straighten(u8 *route, u32 first);

// 0x511470 (cdecl; route, from, limit; name unconfirmed): the walk (LineStep_Init's) from the cell from (its low word) to the route's target (+4),
// the cells kept appended to the route. Each step counts one on 0x7cf3a4 (it and every walker step; at the limit, or the target reached, it ends).
// An open step is taken; after a turn (not the first step since the start or a walk round) the cell is kept when the step on from it the old way is
// open but the new way from there blocked, or (sic) it is blocked and the step just taken open. A blocked step: the cell kept (and, with 0x7cf3c8
// set, the count, the steps and the direction noted at 0x7cf3a0, 0x7cf3b8 and 0x7cf39c), two wall followers (wallfollow.h) sent off from it, the
// one turning back towards the edge first (-1, heading clockwise of the step), then the other; the first back on the line has its cells and its
// own cell appended (0x7cf3bc set to -1 or 1 with 0x7cf3c8 set), the route straightened (Route_Straighten from 0) and the walk set up again from
// its last cell. Failure, 0x70000000 with the cell the walk got to appended: past 0xfa cells kept at a turn, the walkers about to step onto each
// other head on, both given up, the walk's cells too many (the count and them past 0xfc), the limit reached while walking round. The end: with
// under 0xfa cells and the target reached it is appended and the steps returned, else 0x70000000.
u32 __cdecl Route_Search(u8 *route, u32 from, u32 limit);

// 0x511ef0 (cdecl; route, mode; name unconfirmed): sets the mode (0x7cf3c0), empties the route, zeroes the steps (0x7cf3a4) and (with 0x7cf3c8
// set) 0x7cf3a0 to 0xff, then the search from the route's start (+2) with the limit 60000; its result.
u32 __cdecl Route_SearchFrom(u8 *route, u32 mode);
}
