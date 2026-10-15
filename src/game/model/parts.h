// A model's part table: reimplementation of 0x44b140-0x44b406. The model (+4 of the holder
// the functions take, reached through +0 or +8) has +0x48 parts of 0x14 bytes at +0x7c (+0
// flags, +4 and +8 keys) and faces in two arrays: the first +0x44 at +0x70 (0xa0 bytes each),
// the rest at +0x74 (0x58 bytes); a part's face is +0x46 past its index.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x44b140 / 0x44b1b0: flags 0x80000010 on the face of each part whose +8 has any of mask's
// bits / differs from value (the model being (*holder)+4, everything re-read on each part).
void __cdecl Model_MarkFacesByMask(const u8 *const *holder, u32 mask);
void __cdecl Model_MarkFacesNotValue(const u8 *const *holder, u32 value);

// 0x44b220 / 0x44b3c0: the first part (of holder +8 -> +4) whose +4 / +8 is key and whose
// flags have any of mask's bits, or -1. A mask with none of 0x3da1f83 / 0x3da1f82 becomes
// 0x3da1f82 / 1.
u32 __cdecl Model_FindPartByKey4(const u8 *holder, u32 mask, u32 key);
u32 __cdecl Model_FindPartByKey8(const u8 *holder, u32 mask, u32 key);

// 0x44b270: the nth (from 1) part whose flags have any of mask's bits, or -1.
u32 __cdecl Model_NthPart(const u8 *holder, u32 mask, u32 nth);

// 0x44b2b0: how many parts' flags have any of mask's bits.
u32 __cdecl Model_CountParts(const u8 *holder, u32 mask);
}
