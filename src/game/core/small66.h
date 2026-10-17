// One step of a path across the map's cells: reimplementation of 0x52a050. Named by what it does until the caller is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x52a050 (cdecl; the path point {x, y, +0x10 direction byte}, the next x out, the next y out): 0 when the point is outside the map (Map_InBounds); otherwise the cell at the point
// (in the map at [0x7cf83c] + 0x294, 0x44 bytes each, the id (y << 7) + x + 1 cut to 16 bits) gets the neighbour bits of the direction (MapCell_AddNeighbours with Nibble_Scale),
// the next point is the point moved by the direction (1: y + 1, 0x10: y - 1, 4: x - 1, 0x40: x + 1, any other: not moved), 0 again when that is outside; else the cell there is
// returned, and gets the direction (MapCell_SetDirection) when Kinds_Compatible(cell, 3) holds.
u8 *__cdecl Cell_ExtendPath(const s32 *point, s32 *nextX, s32 *nextY);

}  // extern "C"
