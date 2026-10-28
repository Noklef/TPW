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

// 0x4c7520 (thiscall (value), pops 4; called by 0x4d0686 with a thing's +0x118; name invented): the running total +0x21c08 counted up,
// +0x21c40 raised to the value when below it (signed), then message 0x1a (a 0xb, b 1, c and d 0) posted (Object_PostMessage).
void __fastcall ParkAnalyser_Count(u8 *analyser, void *, s32 value);

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

// 0x4c7600 (thiscall (ticks), pops 4; name unconfirmed; called by 0x502357): the whole days (_alldiv by 864000000000, signed; the low
// dword) of the funny time span of `ticks` (FunnyTime_SpanOf of the clock at [[0x7cf6ec]] + 0x2a0) pushed onto the 50-day ring at
// +0x21600: the cursor (+0x216cc) on, back to 0 at the size (+0x216d0, signed) with the wrapped flag (+0x216d4) set; the value at
// +0x21604 + cursor * 4 (not checked).
void __fastcall ParkAnalyser_RecordDays(u8 *analyser, void *, u32 ticks);

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

// 0x4c8ae0 (thiscall on a cell (limit), pops 4; name unconfirmed): as AnalyserCell_CountBusy, the map cells of type 1, 3 or 9
// (MapCell_IsType1, MapCell_IsType3Or9) in the cell's rectangle, kept in its value at +0x4c (+0x4c the ticks, +0x50 the count, +0x54 worked
// out). Returns +0x50.
u32 __fastcall AnalyserCell_CountType1Or3Or9(u8 *cell, void *, u32 limit);

// 0x4c8150 (thiscall (limit), pops 4; name unconfirmed): AnalyserCell_CountType1Or3Or9 of the 0x400 cells (+0x20, 0x7c apart), the last
// first, summed.
u32 __fastcall ParkAnalyser_CountType1Or3Or9(u8 *analyser, void *, u32 limit);

// 0x4c8ba0 (thiscall on a cell (limit), pops 4; name unconfirmed): as AnalyserCell_CountBusy, the map cells in the cell's rectangle (its
// bounds and gMapCells read once) whose dword +0x2c is not 0, kept in its value at +0x58 (+0x58 the ticks, +0x5c the count, +0x60 worked
// out). Returns +0x5c.
u32 __fastcall AnalyserCell_CountWith2c(u8 *cell, void *, u32 limit);

// 0x4c8180 (thiscall (limit), pops 4; name unconfirmed): AnalyserCell_CountWith2c of the 0x400 cells (+0x20, 0x7c apart), the last first,
// summed.
u32 __fastcall ParkAnalyser_CountWith2c(u8 *analyser, void *, u32 limit);

// 0x4c7db0 (thiscall (limit), pops 4; name unconfirmed): the park's score, kept in the value at +0x1f0e8 (+0x1f0e8 the ticks, +0x1f0ec the
// score, +0x1f0f0 worked out) as AnalyserCell_CountBusy's. Worked out again: over the whole map (the world [0x80239c] read once, its cells
// from +0x2d8 for the id (row << 7) + column + 1; the columns 0x7f down to 0, in each the rows 0x7f down to 0) each cell without flag 0x40
// (MapCell_HasFlags) scores 1 for type 2, 3 for 1, 5 for 3 or 9, 1 for 9, 1 for 0xa, 10 for Zero_5d60c0, 2 for 0, 0x32 for
// MapHandle_IsTypeBDOr10Or12Or19; then each thing of the world's chain (+0x1da746, then +0xc; the world read again once) not with +0x19c 4
// (Is19c_Eq4_4e0380) adds (__ftol(+0x44)'s low byte) * the sum of its definition's (ObjectDefs_Find of +0xe, found again each time, not
// checked) dwords +0x1b8 + 0x40 * i for i 0 to +0x50 (at least once) / 100 (signed). Returns +0x1f0ec.
u32 __fastcall ParkAnalyser_ParkScore(u8 *analyser, void *, u32 limit);

// 0x4c7720 (thiscall; name invented, unconfirmed): the analyser's periodic sample, onto its CircBufU32s of 144 (each +4 values, +0x244
// cursor, +0x248 size, +0x24c wrapped; pushed as CircBufU32_144_Push, most inlined in the original): +0x1f100 the tracked source's
// position (Source_Field0c of the table 0x7cfb90's entry for [[0x7cf6ec]]'s word +0x1da726); +0x1f5a0, +0x1f7f0, +0x1fa40, +0x1fc90,
// +0x1fee0, +0x20130, +0x20380 and +0x20cc0 their own +0 (each then cleared); +0x205d0 ParkAnalyser_CountKind1OnPlainCell(0); +0x20820
// the total +0x21c08; +0x20a70 ParkAnalyser_CountDef4ac0(0); +0x20f10 Things_AveragePersonStat19c as a float; +0x1f350
// ParkAnalyser_ParkScore(0), then the tallies +0x21bfc and +0x21c00 cleared; +0x21160 the average of the CircBufU32 of 50 at +0x21600
// (CircBufU32_50_Sum of 0x32: the sum / the count, x87, __ftol's low dword; 0 for no count); +0x213b0 Things_FacilityScore.
void __fastcall ParkAnalyser_Sample(u8 *analyser);

// 0x4c7370 (thiscall (message), pops 4; name invented): on the message's kind (its slot 0): 0xc ParkAnalyser_Sample; 0x11 the id (+8, a
// word) recorded in the u16 ring of its definition's +0x4ac (ObjectDefs_Find): analyser + (that + 0x18d2) * 0x14 (+8 cursor, +0xc size,
// +0x10 wrapped, the values from +2); 0x13 with +8 3 the funny time stamped at +0x21c28; 0x17 with +0xc 2 at +0x21c10; else nothing.
void __fastcall ParkAnalyser_HandleMessage(u8 *analyser, void *, u8 *message);

// 0x4c91c0 (thiscall; name unconfirmed): the people (kind 1) whose +0x220 is 3 in the chains (+0xa, from the cell's first thing, the word
// [0x80239c] + id * 0x44 + 0x2b8) of the two ticket booths' cells (Map_TicketBoothPos 0 and 1 on [0x80239c] + 0x2d8; the same cell counted
// twice when they are one), the world read again for each; the value +0x1f0d0's count +0x1f0d4 = that, +0x1f0d8 = 1, its ticks +0x1f0d0
// the world's (Out_SetFromGame1da70c). Returns +0x1f0d4.
u32 __fastcall ParkAnalyser_CountAtTicketBooths(u8 *analyser);
}
