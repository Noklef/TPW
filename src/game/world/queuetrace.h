// A ride's queue line traced from its entrance (0x530120, RideQueue_Trace; name invented, unconfirmed; called from 0x48d006 for the selected
// object [0x7c2658] and from 0x52610a / 0x527f9a, each when the thing's definition has a queue (+0x40)).
//
// The thing's entrance cell (handle +0x36: the map cell [0x7cf83c] + 0x294 + handle * 0x44, at column (handle - 1) & 0x7f, row
// (handle - 1) >> 7, unsigned) starts the point list (0x81b740: (x, y) pairs, the count at 0x820a8c cleared first; a point is added only
// while the count is at most 0x400, as PairTable_Add). Its neighbour bits (MapCell_Neighbours) give the way out: for 1, 4, 0x10 or 0x40 the
// cell one step on (Map_StepCell by the direction pair 0x7cdba0 / 0x7cdbc8 / 0x7cdba8 / 0x7cdbd0, the listed spot one row / column on
// whatever the pair holds) is, when there is one, either a queue cell (type 3 or 9) of another +0x10 word, which the trace moves onto and
// ends at, or anything else, listed. Otherwise, while the bits are not 0 and the spot is on the map (Map_InBounds), the walk goes on from
// the entrance itself, a cell a step (1: y - 1, 4: x + 1, 0x10: y + 1, 0x40: x - 1; another direction does not move, sic: the same cell
// again, for ever). On each cell:
// - a queue cell at a corner (MapCell_CornerNeighbours) turns the direction (its bits xor the reverse, Nibble_Scale), is listed and gets
//   the overlap counter 1 (MapCell_SetOverlap); any other queue cell gets 0. A cell that is not a queue cell stops the walk and the
//   direction is returned.
// - a cell not of type 9: one of a single neighbour bit stops the walk; one of type 1 (a path) stops it too after cutting it off: the cell
//   a step back (by the direction) loses the direction's bit and gets its model (MapCell_RemoveNeighbours, MapCell_ApplyPathModel) unless it
//   is of type 9 (the walk's spot is left on the cell back then, else put back), and the path cell loses the reverse bit and gets its model.
// With the bits 0 or the spot off the map instead, the entrance cell's direction (MapCell_Direction) turns the thing's facing (+0x10: 0 1,
// 90 4, 180 0x10, 270 0x40; any other angle the low byte of the thing's address, sic) by Byte_ShiftLog2 and the spot moves a step that
// way. The spot where it ends is listed and goes to 0x81ede4 (x) / 0x81ede8 (y). Returns the direction at a cell that is not a queue
// cell, else 0 (in AL; the original leaves the thing's argument slot as the walk's x after a path cell, sic).
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x530120 (cdecl; thing).
u8 __cdecl RideQueue_Trace(const u8 *thing);
}
