// Setting up a walk between two cells: reimplementation of 0x511350.
//
// The arguments are two cell positions, the column in the low byte and the row in the next. The
// walk's globals: 0x7cf3a8 the column distance, 0x7cf3ac the row distance, 0x7cf3c4 whether the
// column distance is the greater (1) or not (0), 0x7cf398 half of each distance added up, and the
// directions 0x7cf3b0 (the column's: 1 for an increase, else 3) and 0x7cf3b4 (the row's: 2 for an
// increase, else 0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x511350 (cdecl; from, to): fills the globals above. With the same column the column direction
// is the row's; with the same row the row direction is the column's (already the row's when the
// columns were also the same).
void __cdecl LineStep_Init(u32 from, u32 to);
}
