// A quicksort of fixed-size records by a dword key, largest first: 0x57d090 and 0x57ced0. Subject and names unconfirmed.
#pragma once

#include "../../../re/types.h"

struct RecordSort {
    u32 stride;      // +0: bytes a record
    u32 keyOffset;   // +4: the unsigned dword key's offset in a record
    u8 *temp;        // +8: room for one record
};

extern "C" {

// 0x57ced0 (thiscall (base, first, last), pops 0xc): records first to last (signed indices, both included) sorted, the largest key first: the middle record's key the pivot, the two ends
// walking in (the left stops at a key not above it, or past last; the right at a key not below it, or at first), swapped through temp when they have not crossed (forward dword then byte
// copies, also a record with itself); the left part recursed into, the right part looped on.
void __fastcall RecordSort_Range(RecordSort *sort, void *, u8 *base, s32 first, s32 last);

// 0x57d090 (thiscall (base, temp, stride, keyOffset, first, last), pops 0x18): the fields set, then RecordSort_Range.
void __fastcall RecordSort_Run(RecordSort *sort, void *, u8 *base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last);

}  // extern "C"
