// A run of type 0x19 map cells marked (0x531040; names unconfirmed): from the cell set from elsewhere (0x7639f0 / 0x7639f4, see focuscell.h), two
// cells a step, starting east (direction 4; Position_StepA's code 0x818c50 set to 4 too). Each cell (the 0x28-byte cell [0x7cf83c] + (id + 0x6cde) *
// 0x28, id = (y << 7) + x + 1 cut to 16 bits) gets flag 0x20 set or cleared (MapCell_Flags / SetFlags / ClearFlags). A type 0x19 cell at a corner
// (MapCell_CornerNeighbours) turns the direction: the reverse one (Nibble_Scale) xor its neighbour bits. The run stops at a cell whose next cell
// two steps on (Map_StepCell28InDirection) is in the map and not of type 0x19, or, after the first, at a cell with one neighbour; else it moves on
// by 2 (1: y - 2, 4: x + 2, 0x10: y + 2, 0x40: x - 2; another direction does not move, sic: the same cell again). A next cell off the map does not
// stop it (sic).
//
// The same run (0x530e80) can list its corners instead: the point list at 0x81b740 ((x, y) pairs of dwords, the count at 0x820a8c, at most 0x401
// points: one is added only while the count is at most 0x400) starts with the start cell; each turn adds the cell it turns at, the end adds the
// cell the run stops at, which also goes to 0x81ede4 / 0x81ede8.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x531040 (cdecl; set: its low byte, set or clear flag 0x20).
void __cdecl CellRun_Mark(u32 set);
// 0x530e80 (cdecl).
void __cdecl CellRun_Corners();
}
