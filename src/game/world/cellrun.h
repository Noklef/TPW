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

// What the runs call in the game that is not ported yet, so that they can run outside it (the benches point it at a recorder).
struct CellRunEnv {
    void(__fastcall *cell53afa0)(u8 *cell);   // 0x53afa0 (agent B's; thiscall on a 0x28-byte cell, no arguments)
    void(__fastcall *cell5370e0)(u8 *cell, void *, u32 a, u32 b, u32 c);   // 0x5370e0 (agent B's; thiscall on a cell (0x81, 0, 0), pops 12)
    void(__fastcall *cell538fc0)(u8 *cell, void *, u32 a);                 // 0x538fc0 (agent B's; thiscall on a cell (0xc), pops 4)
};
// The runs' calls go to env (nullptr: the game's).
void CellRun_SetEnv(const CellRunEnv *env);

// The track's piece shapes along a run from a thing (0x52c6f0 UpdateA, 0x52ccb0 UpdateB; cdecl (thing, flash, start, bank5); names
// unconfirmed): the thing's cell stepped as by AnimateA / AnimateB (a direction other than the right angles: A start's low byte, B
// Nibble_Scale of start), its code to 0x818c50. The run stops at once when the start is not of type 0xb, 0xd, 0x10, 0x12 or 0x19 (A: and
// answers 1). On each track cell (MapCell_IsTypeBOr10; others only move): a corner turns (as AnimateA); else a cell with low flags
// (MapCell_HasLowFlags) gets its shape from the cells two on ahead (a) and behind (b, Map_StepCell28InDirection by the direction and its
// Nibble_Scale): with 2 neighbours, flags 0xf cleared and 4 / 1 / 2 / 8 set (A; B 4 / 1 / 8 / 2) for a and b both with low flags / neither /
// a only / b only; then rules on whether a and b are ends (type 0xb, 0xd, 0x10, 0x12 or 0x19) of the cell's +0x10 word, with one neighbour
// facing them, set the cell 8 or 2 (by whether a is such a type; B's last rule 2 or 8) and then 4 when a or b has low flags (see the
// .cpp); then the env's 0x53afa0 on it. Its thing (+4, entry of 0x7a4610) plays bank 5 (bank5's low byte) or 0xc, entry 0, mode 7, speed 1,
// layer 0 when start's low byte is set (A after the rules, B before them). Then on every track cell, as AnimateA: a cell not at a corner
// whose neighbour bits lack the direction stops the run, and its thing (when not 0) starts flashing (flash's low byte; Highlight_Flash)
// unless its flags (+4) have 0x4000000, or stops (Highlight_Unflash) if they have; a next cell that is an end of the same +0x10 word with the
// reverse side stops it too (A: answers 1 and moves on to it). The run moves on as AnimateA unless stopped. A answers 0 or 1 in AL.
u8 __cdecl TrackRun_UpdateA(u8 *thing, u32 flash, u32 start, u32 bank5);
void __cdecl TrackRun_UpdateB(u8 *thing, u32 flash, u32 start, u32 bank5);

// 0x52fc80 (cdecl (corners); name invented, unconfirmed): the links of the track piece at the cell 0x7639e0 / 0x7639e4 (when Map_InBounds
// and of type 0xb, 0xd, 0x10, 0x12 or 0x19) mended. For each side d = 1, 4, 0x10, 0x40 (Byte_ShiftLog2(d, 4) until back at 1), with the
// cell two on that side (Map_StepCell28InDirection): a side not among its neighbour bits is linked (MapCell_SetDirection Nibble_Scale(d),
// MapCell_AddNeighbours d, the env's 0x53afa0) when that cell is an end (MapCell_IsTypeDOr12ViaParent) of the same +0x10 word; a linked
// side is unlinked (MapCell_RemoveNeighbours d, 0x53afa0) when that cell is neither of the types above nor an end, or of them but without
// the reverse side among its neighbour bits (that cell is not checked for null, sic). Then with corners' low byte set and the cell at a
// corner (MapCell_CornerNeighbours), for each linked side whose cell two on is in the map: one not an end is unlinked both ways (the
// cell's d and its Nibble_Scale(d), 0x53afa0 on each); at an end the cell's direction becomes Nibble_Scale(d) (MapCell_ClearDirection 0xff).
void __cdecl TrackCell_Relink(u32 corners);

// 0x52ba50 (cdecl (x, y, endX, endY, direction, setDirection); name invented, unconfirmed; called from agent B's 0x532585 and 0x53289d): a
// run laid from the cell (x, y) two cells a step until it has been at (endX, endY), from the direction's low byte. The bounds of the cells
// it moves to (not the first) kept: 0x8186e4 / 0x8186e8 the lowest x / y (from 0x400), 0x818c0c / 0x818c08 the highest (from 0; signed).
// At each cell: one not of type 0x19 stops the direction (0) unless the hoardings are placed (byte 0x81b4cc) and it has a neighbour bit
// (MapCell_NeighbourCount). Otherwise a corner (MapCell_CornerNeighbours) turns it: its neighbour bits xor Nibble_Scale of the direction
// when they share a bit, else xor the direction; then a junction (MapCell_JunctionNeighbours) picks the first neighbour side from
// Byte_ShiftLog2(direction, 0x40) on, round by Byte_ShiftLog2(side, 4) (none: it never stops, sic): a side other than the direction becomes
// the cell's direction (MapCell_SetDirection) and the direction's bit and the side's reverse (Nibble_Scale) are taken off its neighbours;
// the same side takes off every bit but it and its reverse (MapCell_RemoveNeighbours); then agent B's unported 0x5370e0 (0x81, 0, 0) and
// 0x53afa0 on the cell (CellRunEnv), and the side is the direction. With setDirection's low byte, a cell whose direction (MapCell_Direction)
// differs gets it, and 0x5370e0 again. The cells one south, east and south-east (Map_StepCell28 by the pairs 0x7cdba8, 0x7cdbc8, 0x7cdbb8,
// the cell's own id; in the map) get the cell's id as parent (MapCell_SetParent) and agent B's unported 0x538fc0 (0xc). Then two cells on
// (1 north, 4 east, 0x10 south, 0x40 west; another direction stays, sic). At the end HoardingGrid_Fill. Returns 1.
u32 __cdecl CellRun_Lay(s32 x, s32 y, s32 endX, s32 endY, u32 direction, u32 setDirection);
}
