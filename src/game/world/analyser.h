// The park analyser (thing model 13: a 32x32 grid of cells and circular-buffer statistics):
// reimplementation of the small methods 0x4c5d10, 0x4c74b0, 0x4c74f0, 0x4c7b70, 0x4c94a0-0x4c94d0
// and 0x4cdb60 (thiscall).
//
// Cells (0x7c bytes) from +0x20, 0x400 of them; four 0x14-byte slots from +0x1f068; byte rings
// of 0x40 bytes from +0x216fc (values at +1, cursor +0x34, size +0x38, wrapped +0x3c); a ring
// of dwords at +0x20824 (cursor +0x20a64, size +0x20a68, wrapped +0x20a6c) holding past values
// of a running total (+0x21c08); tallies at +0x21bfc, +0x21c00 and +0x21c04.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4c5d10 (on a cell): nine trackers of (value 0, best, flag 0) from +0x10, 12 bytes each; the
// first two floats with best -999.0f, the rest ints with best -999.
u8 *__fastcall AnalyserCell_Ctor(u8 *cell);

// 0x4cdb60 (on a slot): u16 +0 = 0, +8 = -1, +0xc = 3, byte +0x10 = 0.
u8 *__fastcall AnalyserSlot_Ctor(u8 *slot);

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
}
