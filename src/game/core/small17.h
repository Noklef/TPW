// Name comparisons through the CRT's _stricmp (core/crt): reimplementation of 0x424cc0, 0x424cf0 and 0x44b2e0. Named by what they do until the
// classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x424cc0 / 0x424cf0 (cdecl; a, b): the texts the dwords at +8 of a and b point to compare, case blind: 1 when the first is after the second / when
// they are equal, otherwise 0.
u32 __cdecl Names_FirstAfter(const u8 *a, const u8 *b);
u32 __cdecl Names_Equal(const u8 *a, const u8 *b);

// 0x44b2e0 (cdecl; model, unused, part): the first of the word at +0x42 elements of 0xa0 bytes (from the block at +0x70 of the structure that +4 of the
// dword at +8 of the model points to; the name pointer at +0x54 of each) whose name equals, case blind, the text for part (1 "mouth - normal",
// 2 "mouth - aah", 3 "mouth - eee", 4 "mouth - ooh", 5 "mouth - sss", -1 "bug head") gives its index; 0 for no match, no elements or any other part.
u32 __cdecl Model_FindMouthPart(const u8 *model, u32 unused, s32 part);

}  // extern "C"
