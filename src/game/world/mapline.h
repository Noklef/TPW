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

// 0x523230 (cdecl; x, y; name unconfirmed): three lines over a path cell and its run. The cell at (x, y) and its entry as MapCell_PathNudge's
// (for an entry of type 0xc or 0x11 the parent's, x and y then the parent cell's (u16 +0 - 1) & 0x7f and >> 7). With flag 0x10 on the entry,
// each matching entry two cells away (as MapCell_PathNudge's; north, east, south, west in turn, a later one overriding) sets the end's offset:
// y -2, x +2, y +2, x -2. The start and the end are then each pushed two cells further apart on each axis where they differ, and
// Map_ApplyLine runs from the start to the end with 0x85, 0x86 and 0x81 (the ends as the lines leave them); the last one's answer.
u32 __cdecl MapCell_PathLines(s32 x, s32 y);

// 0x52a1a0 (cdecl; thing, x out, y out, kind; name unconfirmed): the lines in front of a thing at column +8, row +0xc facing [0x818c4c] (0, 0x5a, 0xb4,
// 0x10e): a start one cell out, an end two further along, Map_ApplyLine with the kind's code (1 -> 0xd, 2 -> 0x12, else the kind), then 0x80, 0x82
// and 0x83, the ends as each leaves them; [0x818c50] the facing's direction bit (4, 1, 0x40, 0x10). An in-bounds start goes to [0x7639e0] /
// [0x7639e4]. The cell four cells out (five for 0x5a / 0xb4) when in bounds: its column and row out, [0x8186cc] the focus cell, the cell
// returned; nullptr otherwise.
u8 *__cdecl Map_ApplyEntranceLines(const u8 *thing, s32 *xOut, s32 *yOut, u32 kind);
}
