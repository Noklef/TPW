// The tag system (thing model 12) and its CTagTable (at +0x80c of the tag system; the name is the one its "not found"
// message gives): reimplementation of the table's lookup 0x50af40. The table itself is static data, 0x17e records of
// 0x20 bytes at 0x75eca8-0x761c68, each starting with its id.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x50af40 (thiscall (id), pops 4; the table object is not read): 0 for the ids 0x17e and 0x17f; the record at
// 0x75eca8 + id * 0x20 (any id, the address wrapping) when its id is id; else the first record of the table with that id,
// or 0 (after a "CTagTable - not found" debug print, not reproduced).
u32 *__fastcall TagTable_Find(void *self, void *, u32 id);
}
