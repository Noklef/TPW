// The advisor's responses: reimplementation of the lookup 0x5997b0 in the response table at 0x768fb8 (entries of 0x20 bytes, ended by
// the id 0x270f), which Advisor_Say (0x599050, "Advisor says ResponseID %d") also walks.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5997b0 (cdecl; id): the table of 0x20-byte entries at 0x768fb8, ended by an id of 0x270f, searched
// for the id; the dword at +0x1c of the entry found, or 0x17f for none.
u32 __cdecl Table768fb8_Lookup(u32 id);

// 0x5989c0 (cdecl; name unconfirmed): the advisor's 0x168-byte block at 0xf79660 zeroed, its count at 0xf79770 set to 5 and 0xf7978c to -1;
// for model slots 0 and 1 that have a holder, the indices of its mouth parts 1 to 5 (Model_FindMouthPart; as many as the count) from
// 0xf796a0 + 0x14 slot and of its "bug head" (-1) at 0xf79698 + 4 slot. Then with slot 1's holder (or slot 0's), the end times of motion bank
// 5's entries 0-19 (Animator_MotionEndMs) from 0xf796c8, the 14th cleared again, and the particle pools reset (ParticlePools_Reset); none of
// that without a holder.
void __cdecl Advisor_Init();

// 0x599e70 (thiscall): the advisor thing's constructor (thing model 11, whose serializer is 0x599f60): the thing constructor (Thing_Construct,
// cell 1, kind 0xb), +0xc and +0x10 cleared, the bytes at +0x28 + 0x18 k (k < 8) cleared, +0xd4 and +0xdc cleared;
// the 0x15f records of 0x10 bytes from +0xe0 get {0, -1, byte 0 (the other three bytes kept), 0}; the byte at +0x16e4 cleared; the advisor
// added to the message centre's slots 5, 0x11, 0x13, 0x17, 0xe, 8 and 0x19 in that order; then +0x10, +0xd4, +0xdc and +0xd8 cleared. Returns
// it. (The cell the original pushes has this's high address bits above the 1.)
u8 *__fastcall Advisor_Ctor(u8 *advisor);
}
