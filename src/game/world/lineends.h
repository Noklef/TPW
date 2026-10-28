// The ends of a line on the map's 0x28-byte cells (names invented, unconfirmed): the cells at 0x7639e0 / 0x7639e4 and 0x7639e8 /
// 0x7639ec (Position_StepA / Position_StepB's, see focuscell.h) and the cells two steps from a cell. Reimplementation of 0x532dd0 and
// 0x53bc60 (cdecl). The cells are the world's ([0x7cf83c] + 0x1102b0 + id * 0x28, id (row << 7) + column + 1): neighbour bits +0xc,
// direction +0xd, flags word +0xe (the low four bits the end flags), count +0x20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x532dd0 (cdecl (cell, pair a, pair b), the pairs of ints by value; name invented, unconfirmed): the cells two steps along the pairs a
// and b (Map_StepCell28 (x, y, 2)), then Map_InBounds of 0x7639e0 / 0x7639e4 and of 0x7639e8 / 0x7639ec (the answers dropped). By the
// cell's direction (MapCell_Direction): 1 or 4 give the flags (2, 8), 0x10 or 0x40 (8, 2), others the first argument's stack slot twice,
// which by then holds the address of the second pair's stack copy (sic; the port gives a local's address, which the benches keep from
// mattering). The cell loses its end flags (MapCell_ClearFlags 0xf). Each found end with end flags (MapCell_HasLowFlags),
// a's with the flags (f, g) and b's with (g, f): having f (MapCell_HasFlags), its end flags become 1; else having 4, they become g;
// either way it gets its model (MapCell_ApplyModel). Last the cell's own model.
void __cdecl TrackCell_ResetEnds(u8 *cell, s32 ax, s32 ay, s32 bx, s32 by);

// 0x53bc60 (cdecl (x, y, dir); name invented, unconfirmed): the world's cell at (x, y); the direction byte d its direction
// (MapCell_Direction). At the end 0x7639e8 / 0x7639ec, d becomes its neighbour bits (MapCell_Neighbours) less the direction, and nothing
// is done when none are left; elsewhere a dir byte other than 0 replaces d, and d 0 skips the cut. The cut: d (the low byte of x's stack
// slot, its other bytes x's, sic) is dropped from the cell's neighbour bits and the cell gets its model; a count (+0x20, MapCell_Overlap)
// other than 0 is counted down (MapCell_DecOverlap); the cell two steps that way (Map_StepCell28InDirection (d, 2)), when there is one,
// loses the reverse (Nibble_Scale) and gets its model. Away from the end: when (x, y) is two cells from the start 0x7639e0 / 0x7639e4 along
// a row or column and the start is in the map, each of its sides 1, 4, 0x10, 0x40 whose cell two steps out (not checked for none) is of
// kind 0xd joins its neighbour bits (MapCell_AddNeighbours) and it gets its model, then the same for kind 0x12. Returns the world (read
// again) plus the offset of the last cell taken: the start's when it was, else (x, y)'s.
u8 *__cdecl LineEnd_Unlink(s32 x, s32 y, u32 dir);
}
