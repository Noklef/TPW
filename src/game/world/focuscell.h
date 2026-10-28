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

// 0x527960 (cdecl (x, y); the caller 0x5244ab passes a third argument, unread; name invented, unconfirmed): the track tool dragged to (x, y).
// With a focus object, its definition (Object_GetDef; none: nothing) sets 0x81b0cc to whether its +0x40 is set and 0x81ecac to its +0x9c
// (both recorded). 0x81ecac (Global81ecac_IfNoFlag) 1, 2 or 4 picks the game mode 0xe, 0x13 or 0x1a, set (GameMode_SetOnly, recorded)
// unless current. With a focus cell (the word 0x818698): the cell (0x7639a4 / 0x7639a8) kept at 0x7639e0 / 0x7639e4 and 0x7639e8 / 0x7639ec,
// the focus object's direction (+0x10, read without a check) at 0x7639ac; a copy stepped by Position_StepA (its code to 0x818c50), the second
// pair by Position_StepB. The point (0x8186b8 / 0x8186b4) is the stepped cell plus the difference to (x, y) with its low bit cleared (rounding
// down). Without a drag start (the word 0x7639b0 -1) the line of the current mode | 0x100 (Map_ApplyLine) from the point to the point
// MapCell_PathNudge moves. A focus object with no +0x2c: TrackRun_TraceA on it, then 0x820afc / 0x820af8 the floats of the ints 0x81ede4 /
// 0x81ede8. Otherwise, with a drag start: the point is the nudged one when that cell's +0x24 word (MapCell_Word24) is above the point's
// (signed), its id the word 0x818684 and its +0x24 word 0x7639b4; unless the cell can take track (MapCell_CanBuildTrack (mode, 0, 0) 9) the
// end is the start (0x818694, 0x7639b0) again and 0x816d48 / 0x816d50 / 0x816d58 cleared. Then CellRun_Connect (no link) from the end to the
// start when the start's word is above the end's (signed), else from the start to the end.
void __cdecl FocusCell_TrackDrag(s32 x, s32 y);
}
