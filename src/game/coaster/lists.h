// Linked lists of 0x10-byte items from a pool, per coaster.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43fcd0: takes the next 0x10-byte item from the pool (cursor at 0x79c604, advanced by
// 0x10) and links it at the list's tail: item +8 next (cleared), +0xc previous; list +4
// count, +8 head, +0xc tail. A list without a slot (+0x10) first gets the next one of the
// current coaster's (0x792218 -> +0x38 -> +0x24 -> +4 = mesh) slots, mesh+0x78 -> +0x6c
// + 0x10 * the counter at 0x7925c8, which is advanced, with its +0xc byte = list flag bit 0;
// mesh+0x3c and (mesh+0x78)+0x5a (words) count it. Returns the item.
u8 *__cdecl CoasterList_Append(u8 *list);
}
