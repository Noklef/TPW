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
}
