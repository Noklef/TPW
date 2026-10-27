// A path cell joined to the cells about it (name invented, unconfirmed): the 0x44-byte map cells (id word +0, kind +8, neighbour bits
// +0xc, direction +0xd, owner word +0x10). Reimplementation of 0x5348d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5348d0 (thiscall (mode, dx, dy, models), pops 0x10; name invented, unconfirmed). Every mode ends with dx and dy stored at 0x763ac0 /
// 0x763ac4.
// - Mode 1 (a path laid): a cell of kind 3 becomes 1, its block value (Map_SetBlockValue of [0x7cf83c] + 0x2d8) the next count of
//   0x7cdb98. Then the cells next to it (Map_StepCell by the pairs from 0x7cdba0) in the order N, NE, E, SE, S, SW, W, NW. A side cell
//   of kind 1 is joined both ways (MapCell_AddNeighbours) and gets each diagonal bit across it whose cell is of kind 1 while the cell
//   that way along the side (Map_StepCellInDirection) is of neither kind 3 nor 9, then its model (MapCell_ApplyPathModel) when models'
//   low byte is set; one (then) of kind 0xa facing back, or of kind 9 facing the cell, is joined the same; one of kind 3 with the reverse
//   among its bits gets its model when models is set and, unless the game is free (0x785a2c), plays the sound 0x8b (Sound_PlayAt in the
//   category [0x803a2c] at x and z the cell's column times the float 0x700d54 (both the column, sic), y 0). A diagonal cell of kind 1
//   with both sides between of kind 1 (not checked for none) is joined both ways and gets its model when models is set. Last, each side
//   direction (Byte_ShiftLog2 (d, 4) from 1 back to 1) among the cell's bits whose cell is of kind 3 or 9 drops the two diagonals beside it.
// - Modes 0 and 3 (a drag): with 0x820aa4 set, each side cell (N, E, S, W) facing back exactly and being the 0x44-byte cell of the id
//   [0x8186ac] +0x36 in [0x7cf83c]'s map is joined both ways, and the step is the one stored at 0x763ac0 / 0x763ac4 (0, 0 when either is
//   0x80); else dx, dy. dy 1, dx -1, dy -1, dx 1 look N, E, S, W respectively: a cell there of the kind mode with under two neighbours
//   (MapCell_NeighbourCount, signed) and the owner of the word 0x818698 or none is joined both ways; a cell of the kind mode there makes
//   a cell of kind 1 drop the two diagonals beside that side.
// - Others: nothing more.
void __fastcall MapCell_LinkPath(u8 *cell, void *, u32 mode, s32 dx, s32 dy, u32 models);
}
