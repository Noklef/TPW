// The list of rides' records and its accessors: reimplementation of 0x55a070, 0x55a0b0, 0x55a300, 0x55a330, 0x55a360, 0x55a390 and 0x55a3e0.
//
// The dword at 0x8791a0 says the ride system exists (the accessors report to the empty debug printf and give 0 otherwise); the dword at 0x8791b0
// heads a list of records (the next at +0, an id at +8, a link id at +0x14, an array of dwords at +0x1c of the count at +0x8c, a word at +0xc0, a
// dword at +0x9c). The debug printf calls of the original are left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x55a070 (cdecl; id): the record with the id, 0 when there is none or no system.
u8 *__cdecl Rides_FindById(s32 id);

// 0x55a0b0 / 0x55a390 (cdecl; record, index[, value]): the dword at that index of the record's array, set / read, when the record is not 0 and
// the index is within its count (0 if not).
void __cdecl Rides_SetElement(u8 *record, s32 index, u32 value);
u32 __cdecl Rides_GetElement(const u8 *record, s32 index);

// 0x55a300 / 0x55a330 / 0x55a360 (cdecl; record[, value]): the word at +0xc0 set from the low 16 bits of the value / read, sign extended / the
// dword at +0x9c; 0 for no record or no system.
void __cdecl Rides_SetWordC0(u8 *record, u32 value);
s32 __cdecl Rides_GetWordC0(const u8 *record);
u32 __cdecl Rides_Get9c(const u8 *record);

// 0x55a3e0 (cdecl; id, index): the element of the array of the record that the link id (+0x14) of the record with the id names; 0 if either
// record is missing or the index is outside.
u32 __cdecl Rides_GetLinkedElement(s32 id, s32 index);

// Objects that keep the id of their ride record at +0x24: 0x4e03f0 (thiscall) sets its element 4 to 0; 0x4e0410 whether its element 7 is above 0
// (signed); 0x4e0a70 whether its element 0 differs from the word at +0x3c.
void __fastcall Obj_ClearRideElement4(const u8 *object);
u32 __fastcall Obj_RideElement7Positive(const u8 *object);
u32 __fastcall Obj_RideElement0Differs(const u8 *object);

}  // extern "C"
