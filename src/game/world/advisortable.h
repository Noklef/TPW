// The table of the advisor's message ids at 0x76e300 that AdvisorScoring_Score reads: 351 records of 0x38 bytes, filled at start-up. Reimplementation of 0x5a0a50.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5a0a50 (31916 bytes of stores, called through the static initialiser 0x5a0a40; name unconfirmed): each record i: +0 a flag, +4 the id i, +8 a value, +0xc when it
// applies (see advisorscore.h), +0x10 the method (a game address), +0x14..+0x1c its pointer to member's other parts (0), +0x20, +0x24, +0x28 values, +0x2c 2, +0x30 0x160.
// +0x34 is not written, nor are record 0's first four fields.
void AdvisorTable_Fill();
}
