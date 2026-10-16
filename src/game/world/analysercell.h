// The park analyser's cells: reimplementation of 0x4fa990.
//
// [0x8023a0] points at 0x44-byte cells, 128 to a row; an analysed record keeps its cell's column at
// +5 and row at +7.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4fa990 (thiscall on the record): 1 when the cell at its column and row has type (the dword at
// +8; MapCell_IsType1/3Or9/9/A/0) 0, 1, 3, 9 or 0xa, else 0. The index is the row * 128 + column,
// taken to 16 bits.
u32 __fastcall AnalyserCell_TypePlain(const u8 *record);
}
