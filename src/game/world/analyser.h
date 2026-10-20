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
}
