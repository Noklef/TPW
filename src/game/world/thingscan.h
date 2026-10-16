// Scans over the park's things (the thing table, world/things): reimplementation of 0x4c9f90, 0x4ca0c0 and 0x4ca820. Each walks the in-use
// list from the newest thing (World_FirstThing / World_NextThing on the world at 0x80239c) and looks at the kind byte at +2 of each (3 is
// the kind these look for). The original also calls its empty debug printf for every thing, which is left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4c9f90 / 0x4ca0c0 (cdecl): how many things of kind 3 have bit 0 / bit 1 of the byte at +0x32 set and a dword at +0x68 that is 0.
s32 __cdecl Things_CountKind3Flag1();
s32 __cdecl Things_CountKind3Flag2();

// 0x4ca820 (stdcall; value, pops 4): the greatest byte at +0x50 over the things of kind 3 whose word at +0xe equals the low 16 bits of the
// value (-1 for none).
s32 __stdcall Things_MaxKind3Byte50(u32 value);

// 0x4828f0 (cdecl; value, previous): the first thing after the previous one (the newest when it is 0) that is an object (kind 3) whose
// definition's word at +0x284 (Object_DefWord284) is 0 and whose definition's dword at +0x4ac equals the value; 0 for none.
u8 *__cdecl Things_FindNextByDef4ac(s32 value, const u8 *previous);

// The same walk, after the previous thing, for the first object whose definition's word at +0x284 is not 0 (0x4829c0); or also with the
// definition's dword at +0x288 equal to 2 (0x482a80); or with the word 0, the dword at +0x4ac equal to 3 and bit 0 of its byte at +0x32
// set (0x482b50) / bits 0 and 1 both clear (0x482c20); or simply with bit 1 of its byte at +0x32 set (0x482cf0).
u8 *__cdecl Things_FindNextWithDefWord(const u8 *previous);
u8 *__cdecl Things_FindNextWithDefWordUpgrade2(const u8 *previous);
u8 *__cdecl Things_FindNextShopFlag1(const u8 *previous);
u8 *__cdecl Things_FindNextShopNoFlags(const u8 *previous);
u8 *__cdecl Things_FindNextKind3Flag2(const u8 *previous);

// 0x4831f0 (cdecl; out): the handle of the oldest thing to *out (Thing_GetHandle; with no thing at all the original reads through a null
// pointer). Returns out.
u16 *__cdecl Things_LastHandle(u16 *out);

// 0x4ca030 (cdecl): how many things of kind 3 have bit 1 of the byte at +0x32 set.
s32 __cdecl Things_CountKind3Flag2Any();

// 0x4c9960 (stdcall; kind): how many staff things (kinds 4 to 8) there are of that kind whose byte at +3 is 0.
s32 __stdcall Things_CountStaffOfKind(u32 kind);

// 0x4c9c00 (stdcall; kind): over the things of that kind byte (at +2), the sum of their dwords at +0x1e4 times 100 over 5 times their
// number (0 for none).
s32 __stdcall Things_AverageOfKind(u32 kind);

// 0x4c9ee0 (thiscall on the game object): +0x1f0e0 = how many things of kind 3 have bit 0 of the byte at +0x32 set, +0x1f0e4 = 1, and
// +0x1f0dc set from the game time (Out_SetFromGame1da70c). Returns the count.
u32 __fastcall Game_RecountKind3Flag1(u8 *game);

// 0x4ca8e0 (stdcall; value, least): how many things of kind 3 have a definition whose dword at +0x4ac is the value, and, when the value is
// 0, a byte at +0x50 that is at least the least (signed).
s32 __stdcall Things_CountByDef4ac(s32 value, s32 least);

// 0x4ca9b0 (stdcall; value): the same without the least, for the things whose dword at +0x68 is 0.
s32 __stdcall Things_CountByDef4acIdle(s32 value);

}  // extern "C"
