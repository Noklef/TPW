// The challenges (thing model 19: 0x14 challenges of 0x30 bytes from +0xc, the current one's
// index at +0x3cc) and the BalanceChallenges file they load from (vtable 0x700398):
// reimplementation of 0x4d1c80, 0x4d1e60 and 0x4d2840 (thiscall).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d1c80: each challenge in use (+0 set) that names another (+0x18 set) is dropped (+0 = 0)
// unless some other challenge in use has that +0, as the list stands at that point.
void __fastcall Challenges_DropOrphans(u8 *challenges);

// 0x4d2840: the current challenge (index +0x3cc, 0-19 signed), or null.
u8 *__fastcall Challenges_Current(u8 *challenges);

// 0x4d1e60 (the vtable's first slot; index): the schema of a field, 0x739b98 + index * 0x3c;
// this is unused.
const u8 *__fastcall BalanceChallenges_GetSchema(const u8 *file, void *, u32 index);
}
