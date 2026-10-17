// A straight run of map cells (0x53aac0): from (x, y) along the axis of the larger distance to
// (*xEnd, *yEnd) (the y axis on a tie), two cells a step. The end on that axis is first pulled
// in to an even distance (a positive odd distance is rounded up, then the distance halved and
// doubled, toward zero) and written back. Each cell of the run in the map (Map_InBounds), the
// end included, gets the cell operation 0x5370e0 (on the 0x28-byte cell [0x7cf83c] + (id +
// 0x6cde) * 0x28, id = (y << 7) + x + 1 cut to 16 bits) with (arg, x step, y step), the step
// on the other axis 0. A cell operation that answers 0 stops the run: the answer is then 0,
// else 1 (also when the end is outside the map).
//
// The globals gLineFirst (0x820ac8: 1 until the first cell operation has run) and gLineLast
// (0x820ad0: 1 from the end cell's turn) tell the cell operation where it is; both are 0 after
// the end cell's operation, but an end outside the map leaves gLineLast 1 and gLineFirst as it
// was (1 when no cell operation ran).
//
// The originals' quirks are kept:
// - The loop compares x (or y) against *xEnd (*yEnd) read again after every cell, but the first
//   test is against the value it wrote.
// - The distances are 32-bit (abs of the most negative int is itself).
//
// A line's cell linked back to the one before (0x539220, a cell operation of another line, its kind from 0x816d64; name unconfirmed): the step
// is (stepX, stepY), or on the first cell (gLineFirst) the last one kept at 0x763b00 / 0x763b04 (0 when either is 0x80). For a kind of 0xb, 0xd,
// 0x10 or 0x12 (track) or 0x19, each side the step came from (y step 2: north, x step -2: east, y step -2: south, x step 2: west, in that order)
// is linked: the cell two away that way (Map_StepCell28 by the direction pair at 0x7cdba0 + ...) of one of those types (only 0x19 for kind 0x19),
// both cells with fewer than 4 neighbour bits and its owner (+0x10) the focus cell's id (0x818698) or 0, gets the reverse side's neighbour bit
// and the cell that side's. The step is then kept at 0x763b00 / 0x763b04.
#pragma once

#include "../../../re/types.h"

struct MapLineEnv {
    u32(__fastcall *cellOp)(u8 *cell, void *, u32 arg, s32 stepX, s32 stepY);  // 0x5370e0 (pops 12)
};
// nullptr restores the game's.
void MapLine_SetEnv(const MapLineEnv *e);
const MapLineEnv *MapLine_GetEnv();

extern "C" {
// 0x53aac0 (cdecl): 1, or 0 when a cell operation answered 0.
u32 __cdecl Map_ApplyLine(u32 arg, s32 x, s32 y, s32 *xEnd, s32 *yEnd);
// 0x539220 (thiscall on the cell; kind, step, pops 12): see above.
void __fastcall MapLine_LinkCell(u8 *cell, void *, u32 kind, s32 stepX, s32 stepY);
}
