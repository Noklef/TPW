// The focus cell (names unconfirmed): reimplementation of its update 0x5276d0.
//
// Its globals: 0x818698 the focus cell's id (a word, (row << 7) + column + 1), 0x8186a0 the one before, 0x8186ac the object in focus, 0x8186c4
// the cell's entry in [0x7cf83c]'s table (0x28 bytes each, from id + 0x6cde), 0x7639a4 / 0x7639a8 / 0x7639ac its column, row and direction,
// 0x7639e0 / 0x7639e4 and 0x7639e8 / 0x7639ec the cell stepped by Position_StepA and Position_StepB, 0x818c50 Position_StepA's code, 0x7639f0 /
// 0x7639f4 a cell set from elsewhere (-1 for none).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5276d0 (cdecl; record): the old id kept and the id cleared, the column, row and direction -1. In game mode (0x81ae2c) 0x19 or 0x1a with a set
// cell: that cell, direction 0, code 4. Otherwise the selected object (0x7c2654; nothing more when none) is the focus, or, when its handle word
// (+0x30, Fn_4e0df0) names a thing whose definition (ObjectDefs_Find of the object's id +0xe) has +0x288 1, that thing (which becomes the
// selected object): its cell (+5 column, +7 row), its direction (+0x10) and Position_StepA's code. Then Position_StepB by the direction. With a
// focus object, an id unlike the old one and the record flag's low byte set, the action 0x32 with the focus object's cell id is recorded
// (ActionRec_WriteW). The debug check on the thing table is left out.
void __cdecl FocusCell_Update(u32 record);
}
