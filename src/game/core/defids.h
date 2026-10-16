// The table of object definitions the code looks things up in by id: 0x38-byte records from 0x76e300, the id at +4 of each. (Used by the small
// getters of its fields; the lookup is not a function of the game of its own, it is inlined in each of them.)
#pragma once

#include "../../../re/types.h"

// The record for the id: the one at id * 0x38 when its id is the id, and also (sic: the original returns the record by index, not the
// one it found) when the id is found anywhere in the table; nullptr for an id that is not there and for 0x15f and 0x160, which are never
// looked for. (The original reports a miss to its debug printf, which does nothing.)
const u8 *ObjectDef_FindById(u32 id);
