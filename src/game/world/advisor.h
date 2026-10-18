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
}
