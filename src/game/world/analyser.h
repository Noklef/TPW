// The park analyser (thing model 13: a 32x32 grid of cells and circular-buffer statistics):
// reimplementation of the small methods 0x4c5d10, 0x4c74b0, 0x4c74f0, 0x4c7680, 0x4c76d0, 0x4c7b70 and
// 0x4c94a0-0x4c94d0 (thiscall).
//
// Cells (0x7c bytes) from +0x20, 0x400 of them; four CircBufU16s of 3 from +0x1f068; CircBufU8s
// of 50 from +0x216fc (values at +1, cursor +0x34, size +0x38, wrapped +0x3c); a CircBufU32
// of 144 at +0x20820 (values from +0x20824, cursor +0x20a64, size +0x20a68, wrapped +0x20a6c)
// holding past values of a running total (+0x21c08); tallies at +0x21bfc, +0x21c00, +0x21c04.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x4c5d10 (on a cell): nine trackers of (value 0, best, flag 0) from +0x10, 12 bytes each; the
// first two floats with best -999.0f, the rest ints with best -999.
u8 *__fastcall AnalyserCell_Ctor(u8 *cell);

// 0x4c74b0 (ring, value): the ring's cursor on (back to 0 at its size, signed, setting the
// wrapped flag), and the value's low byte stored there.
void __fastcall ParkAnalyser_Record(u8 *analyser, void *, u32 ring, u32 value);

// 0x4c9920 (ring; name unconfirmed): whether the ring holds 50 values: wrapped (+0x3c), its size +0x38 at least 50, else its cursor
// +0x34 + 1 at least 50 (signed). In AL only.
u8 __fastcall ParkAnalyser_RingFull(const u8 *analyser, void *, u32 ring);

// 0x4c9830 (ring, pops 4; name unconfirmed): the average of the ring's newest values, at most 50 (wrapped: its size +0x38, else the
// cursor +0x34 + 1; signed), from the cursor back (r[cursor - i + 1], past the cursor only once wrapped: r[cursor - i + size + 1]; none
// at or past the size), truncated (__ftol); 0 when there are none. In AL only (callers keep the byte). The original's debug message for a
// size of 50 and up is gone.
u8 __fastcall ParkAnalyser_RingAverage(const u8 *analyser, void *, u32 ring);

// 0x4c74f0 (flag): +0x21bfc and +0x21c04 counted up, +0x21c00 too when flag is 0.
void __fastcall ParkAnalyser_Tally(u8 *analyser, void *, u32 flag);

// 0x4c94a0 (flag, its low byte): +0x21c00 when set, else +0x21bfc.
u32 __fastcall ParkAnalyser_Tallied(const u8 *analyser, void *, u32 flag);

// 0x4c94c0: the running total, +0x21c08.
u32 __fastcall ParkAnalyser_Total(const u8 *analyser);

// 0x4c94d0 (back): the total less its value back entries ago in the ring (signed compares;
// before the cursor only once wrapped), or 0 when there is none.
u32 __fastcall ParkAnalyser_TotalSince(const u8 *analyser, void *, s32 back);

// 0x4c7b70 (pos, its low 16 bits): the cell of pos - 1 (a 128-wide grid): (row / +0xc) *
// +0x10 + (column / +0xc), signed divisions.
u8 *__fastcall ParkAnalyser_Cell(u8 *analyser, void *, u32 pos);

// 0x4c7680 / 0x4c76d0 (names unconfirmed): the funny time now (FunnyTime_Now of the world's clock, [0x80239c] + 0x2a0) to the 8 bytes at
// +0x21c30 / +0x21c38 (their callers: 0x4db7a3, and the research lab's 0x504c65).
void __fastcall ParkAnalyser_StampTime21c30(u8 *analyser);
void __fastcall ParkAnalyser_StampTime21c38(u8 *analyser);

// 0x4c7a90 (thiscall; name unconfirmed): the analyser added to the message centre's slots 0xc, 0x11 and 0x13, then each of its +0x14 * +0x10
// cells (rows * columns, a 32-bit product; none when not positive; 0x7c bytes from +0x20) gets its rectangle: x = column * size, y = row * size,
// w and h = size (+0xc), cut back where the cell passes the width (+0x18) or the height (+0x1c): w = width - (column + 1) * size + size. Column
// and row are cell % columns and cell / columns, signed; the products wrap. Returns 1.
u32 __fastcall ParkAnalyser_InitCells(u8 *analyser);

// 0x4c58c0 (thiscall; the constructor): Thing_Construct (cell 1, kind 0xd); the 0x400 cells (AnalyserCell_Ctor from +0x20); the four CircBufU16s of
// 3 (+0x1f068); six trackers (0, -999, flag 0; 12 bytes from +0x1f0b8); inlined CircBufU32s (value +0 = 0, then after the values cursor -1, size,
// wrapped 0): sixteen of 144 from +0x1f100 (0x250 apart), one of 50 at +0x21600 and one of 5 at +0x216d8; twenty CircBufU8s of 50 (+0x216fc);
// cell size +0xc 4, columns and rows +0x10 / +0x14 32, width and height +0x18 / +0x1c 128; the tallies and total +0x21bfc-+0x21c08 0; the six
// time stamps +0x21c10-+0x21c38 the funny time now (as ParkAnalyser_StampTime21c30); +0x21c40-+0x21c48 0; then ParkAnalyser_InitCells. Returns it.
u8 *__fastcall ParkAnalyser_Ctor(u8 *analyser);

// 0x4cd9c0 (thiscall (stream, save, version, unused), pops 16): an analyser value, as a serializer does (Save_Field): mVal (+4, 4 bytes)
// and mWorkedOut (+8, 1). 1 at the first that fails (the debug print left out), else 0; 0 for a version outside 0..1.
u32 __fastcall AnalyserVal_Serialize(u8 *val, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4c85b0 (thiscall (stream, save, version, unused), pops 16): an analyser cell (AnalyserCell_Ctor's 0x7c bytes). Saving: the four dwords +0-+0xc
// (Save_Field), the first six values (+0x10, 0xc apart) inline as Save_WriteU32 of mVal and Save_WriteU8 of mWorkedOut, the last three
// (+0x58, +0x64, +0x70) through AnalyserVal_Serialize. Loading: +0 (Save_Field), +4-+0xc (Save_ReadU32), then all nine values
// (AnalyserVal_Serialize, save 0). 1 at the first that fails (the debug print left out), else 0; 0 for a version outside 0..1.
u32 __fastcall AnalyserCell_Serialize(u8 *cell, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4c5d70 (thiscall (stream, save, version, unused), pops 16): the park analyser. Thing_Serialize first (1 when it fails); then, for versions
// 0..1 only: +0xc-+0x1c (saved as Save_Field, loaded by Save_ReadU32), the 0x400 cells (from +0x20), the four CircBufU16s (+0x1f068, 0x14
// apart), the six values (+0x1f0b8), the sixteen CircBufU32s of 144 (+0x1f100, 0x250 apart), the one of 50 (+0x21600) and of 5 (+0x216d8),
// the twenty CircBufU8s (+0x216fc, 0x40 apart), each through its serializer (saving, the original inlines some: the same calls), then
// +0x21bfc-+0x21c08, the six funny times +0x21c10-+0x21c38 (8 bytes) and +0x21c40-+0x21c48 by the Save_Write / Save_Read helpers (loading,
// +0x21c04 is written, sic). 1 at the first that fails, else 0.
u32 __fastcall ParkAnalyser_Serialize(u8 *analyser, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4cd880 (thiscall on the analyser, not read (cell id, a word; radius), pops 8; name unconfirmed): the people (things of kind 1) in the
// map's cells (gMapCells [0x8023a0], each one's first thing the word +0x24, then each thing's +0xa) within radius columns and rows of the
// cell id (column (id - 1) & 0x7f, row (id - 1) >> 7, unsigned; the far edges cut at 0x7f as unsigned sums, the near ones signed and not
// cut, the cells' ids (row << 7) + column + 1 as words): the average (signed division) of the low bytes of __ftol of their floats +0x19c;
// 0 for none.
s32 __fastcall ParkAnalyser_AveragePersonStat19c(void *self, void *, u32 cell, u32 radius);

// 0x4c8c40 (thiscall on a cell (limit), pops 4; name unconfirmed): the count of busy map cells in the cell's rectangle, kept in its value
// at +0x64 (+0x64 the ticks it was worked out at, +0x68 the count, +0x6c worked out): kept while worked out and limit is at least its age
// in quarters (Game1da70c_MinusAt0Quarters, unsigned); else counted again over the map cells (gMapCells [0x8023a0], 0x44 bytes, the id
// (row << 7) + column + 1 as a word) of the columns +0 up to +0 + +8 and the rows +4 up to +4 + +0xc (signed, the bounds read again at
// each step) whose type is 1, 3 or 9, 9, 0xa or 4 (MapCell_IsType1, MapCell_IsType3Or9, MapCell_IsType9, MapCell_IsTypeA, Zero_5d60c0,
// MapCell_IsType4), and +0x68 = that, +0x6c = 1, +0x64 the world's ticks (Out_SetFromGame1da70c). Returns +0x68.
u32 __fastcall AnalyserCell_CountBusy(u8 *cell, void *, u32 limit);

// 0x4c81b0 (thiscall (limit), pops 4; name unconfirmed): AnalyserCell_CountBusy of the 0x400 cells (+0x20, 0x7c apart), the last first,
// summed.
u32 __fastcall ParkAnalyser_CountBusy(u8 *analyser, void *, u32 limit);

// 0x4c91c0 (thiscall; name unconfirmed): the people (kind 1) whose +0x220 is 3 in the chains (+0xa, from the cell's first thing, the word
// [0x80239c] + id * 0x44 + 0x2b8) of the two ticket booths' cells (Map_TicketBoothPos 0 and 1 on [0x80239c] + 0x2d8; the same cell counted
// twice when they are one), the world read again for each; the value +0x1f0d0's count +0x1f0d4 = that, +0x1f0d8 = 1, its ticks +0x1f0d0
// the world's (Out_SetFromGame1da70c). Returns +0x1f0d4.
u32 __fastcall ParkAnalyser_CountAtTicketBooths(u8 *analyser);
}
