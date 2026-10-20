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
//
// A track run from a thing (0x530830 TraceA, 0x530b70 TraceB; names unconfirmed): the thing's cell (+5 column, +7 row, as a cell id: the column's
// low 7 bits, the row plus its top bit) stepped by its direction (+0x10) with Position_StepA (A) or Position_StepB (B), whose code is the run's
// direction (and 0x818c50). The point list is cleared and gets the start when it is a track cell (MapCell_IsTypeBOr10). Then on each cell, two
// cells a step: a track cell at a corner turns (as above), is listed and gets the overlap counter 1 (MapCell_SetOverlap); else 1 at a junction, 0
// otherwise; B then sets it to 0 on the cell whose id is the one at 0x7639e0 / 0x7639e4 (when that is in the map, else id 0). A track cell not at a
// corner whose neighbour bits lack the direction stops the run; so does a next cell (two on, Map_StepCell28InDirection) that is an end
// (MapCell_IsTypeDOr12ViaParent) of the same +0x10 word with the reverse side among its neighbour bits, A moving on to it first. The run moves on
// (as above) unless stopped; other cells only move. B stops after the first cell when the start is not of type 0xb, 0xd, 0x10, 0x12 or 0x19. The
// end is listed and goes to 0x81ede4 / 0x81ede8.
//
// The track run animated (0x52a490 AnimateA, 0x52a700 AnimateB; names unconfirmed; called with [0x8186ac] from 0x524d29, 0x526bef, 0x526f62,
// 0x554a1c): from the thing's cell stepped as by TraceA / TraceB (Position_StepA / StepB inlined; a direction other than the right angles gives
// the code start's low byte, B's through Nibble_Scale), on each cell: one of type 0xb, 0xd, 0x10, 0x12 or 0x19 (MapCell_IsTypeBDOr10Or12Or19)
// turns at a corner (the reverse direction xor its neighbour bits, or the direction itself xor them when the reverse is not among them) and has
// its thing (+4, when not 0: entry of 0x7a4610) play bank 5 (start's low byte set) or 0xc, entry 0, mode 7, speed 1, layer 0 (Animator_Play);
// any other cell sets the direction to 0. A cell not at a corner whose neighbour bits lack the direction stops the run, as does a next cell two
// on that is an end of the same +0x10 word with the reverse side (as TraceB); the run moves on (as above) either way. The direction 0 on a
// corner cell of no such type never moves and never stops (sic). Returns the last direction less 1 with the low byte cleared (the original's
// EAX: 0xffffff00 for 0, else 0; the callers ignore it).
//
// The point list's own add and commit (0x52fbf0, called from 0x524800-0x526600; 0x52fc30, called from 0x48d013): the runs above add inline.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x531040 (cdecl; set: its low byte, set or clear flag 0x20).
void __cdecl CellRun_Mark(u32 set);
// 0x530e80 (cdecl).
void __cdecl CellRun_Corners();
// 0x530830 / 0x530b70 (cdecl; thing).
void __cdecl TrackRun_TraceA(const u8 *thing);
void __cdecl TrackRun_TraceB(const u8 *thing);
// 0x52a490 / 0x52a700 (cdecl; thing, start).
u32 __cdecl TrackRun_AnimateA(const u8 *thing, u32 start);
u32 __cdecl TrackRun_AnimateB(const u8 *thing, u32 start);

// 0x52fbf0 (cdecl; a, b): appends the pair to the table of 8-byte entries at 0x81b740 (count at
// 0x820a8c), unless the count is above 0x400 (signed): 0 then, else 1.
u32 __cdecl PairTable_Add(u32 a, u32 b);

// 0x52fc30 (cdecl): the pair table at 0x81b740 (count 0x820a8c) copied to 0x818d98; then 0x820a90 = the
// count and 0x820a94 = 0. Returns the count when above 0, else 0.
s32 __cdecl PairTable_Commit();
}
