// Quicksorts of fixed-size records and of pointers to records by a key: 0x57d090, 0x57ced0, 0x57cda0, 0x57d060, 0x57cbc0, 0x57d000, 0x57ccb0, 0x57d030. Subject and names unconfirmed.
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

// 0x57cda0 / 0x57d060: as RecordSort_Range / RecordSort_Run, the smallest key first.
void __fastcall RecordSort_RangeUp(RecordSort *sort, void *, u8 *base, s32 first, s32 last);
void __fastcall RecordSort_RunUp(RecordSort *sort, void *, u8 *base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last);

// 0x57cbc0 / 0x57d000: an array of pointers to records sorted by the records' unsigned dword key, the smallest first; the same walks, each swap through the sort's +8, which keeps the last pointer so moved.
void __fastcall RecordSort_PointersUp(RecordSort *sort, void *, u8 **base, s32 first, s32 last);
void __fastcall RecordSort_RunPointersUp(RecordSort *sort, void *, u8 **base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last);

// 0x57ccb0 / 0x57d030: the same by a float key, the largest first; the left stops at an unordered key, the right walks past it.
void __fastcall RecordSort_PointersByFloat(RecordSort *sort, void *, u8 **base, s32 first, s32 last);
void __fastcall RecordSort_RunPointersByFloat(RecordSort *sort, void *, u8 **base, u8 *temp, u32 stride, u32 keyOffset, s32 first, s32 last);

}  // extern "C"
