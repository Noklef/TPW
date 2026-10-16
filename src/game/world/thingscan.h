// Scans over the park's things (the thing table, world/things): reimplementation of 0x4c9f90, 0x4ca0c0 and 0x4ca820. Each walks the in-use
// list from the newest thing (World_FirstThing / World_NextThing on the world at 0x80239c) and looks at the kind byte at +2 of each (3 is
// the kind these look for). The original also calls its empty debug printf for every thing, which is left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4c9f90 / 0x4ca0c0 (cdecl): how many things of kind 3 have bit 0 / bit 1 of the byte at +0x32 set and a dword at +0x68 that is 0.
s32 __cdecl Things_CountKind3Flag1();

// 0x4c9a10 (cdecl): the things of kind 4 to 8 (the staff).
s32 __cdecl Things_CountStaff();

// 0x4c8430 (cdecl; three byte outputs, pops 12): over the things of kind 1 whose analyser cell is plain (AnalyserCell_TypePlain), the three values of the dword at +0x30
// (0 to 22; not checked) that occur most, counting from value 3 up and starting from {0, 1, 2} (the count of 0 forced to 0): a later value replaces the entry it beats by the
// most (a strictly larger gain). Each output is the entry, or 0x17 when its count is 0.
void __cdecl Things_TopThreeValues(u8 *first, u8 *second, u8 *third);

// 0x4c8d30 (cdecl; kind, cell id, radius, first only, pops 16): the things of the kind (the byte at +2) in the lists of the cells within the radius (clamped to 0 to 0x7f)
// of the cell: the list head is the word at -0x20 of each 0x44-byte cell, the links the words at +0xa of each thing. Returns 1 at the first one when first only is set, else
// the count.
s32 __cdecl Things_CountOfKindAround(u32 kind, u32 cell, s32 radius, s32 firstOnly);
// 0x519690 / 0x519800 / 0x519970 / 0x519c50 / 0x519ae0 (thiscall on the game object; thing, pops 4): the thing is taken out of the singly linked list of
// things whose head handle is the word at +0x1da73c / +0x1da73e / +0x1da740 / +0x1da742 / +0x1da744 and whose links are the words at +0x210 of each: when it is
// the head the head becomes its link, otherwise the link of the thing before it becomes its link (found by walking from the head; a thing that is not in the
// list runs the walk off its end). 0x519dc0 is the same for the head at +0x1da746 and the links at +0xc.
void __fastcall Things_Unlink1da73c(u8 *game, void *, const u8 *thing);
void __fastcall Things_Unlink1da73e(u8 *game, void *, const u8 *thing);
void __fastcall Things_Unlink1da740(u8 *game, void *, const u8 *thing);
void __fastcall Things_Unlink1da742(u8 *game, void *, const u8 *thing);
void __fastcall Things_Unlink1da744(u8 *game, void *, const u8 *thing);
void __fastcall Things_Unlink1da746(u8 *game, void *, const u8 *thing);
// 0x4c9e60 (cdecl): from the thing the word at +0x1da742 of the game object [0x80239c] names, along the chain of the word at +0x210 of each, the sum of the
// dwords of the table at 0x78545c (12 bytes each) indexed by the dword at +0x1e4 of each thing.
u32 __cdecl Things_SumChain();
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

// The same finders going the other way: from the thing before the previous one, or from the oldest thing when the previous is 0 (which, with
// no things at all, reads through a null pointer in the original): 0x483280 (value, previous) for the object whose definition's word at +0x284
// is 0 and dword at +0x4ac the value; 0x4833c0 for a non-zero word; 0x4834e0 for that and the definition's dword at +0x288 equal to 2;
// 0x483620 for an object with bit 1 of its byte at +0x32 set.
u8 *__cdecl Things_FindPrevByDef4ac(s32 value, const u8 *previous);
u8 *__cdecl Things_FindPrevWithDefWord(const u8 *previous);
u8 *__cdecl Things_FindPrevWithDefWordUpgrade2(const u8 *previous);
u8 *__cdecl Things_FindPrevKind3Flag2(const u8 *previous);

// 0x4c9130 / 0x4c9530 / 0x4c95d0 (cdecl): how many things of kind 1 are on a plain analyser cell (AnalyserCell_TypePlain) and, for the
// second, have a dword at +0x210 that is not 0, for the third, a dword at +0x24 that is 2.
s32 __cdecl Things_CountKind1OnPlainCell();
s32 __cdecl Things_CountKind1OnPlainCellWith210();
s32 __cdecl Things_CountKind1OnPlainCellMode2();

// 0x59edd0 / 0x59ee20 / 0x59ee70 / 0x59eec0 (thiscall on an object with the limits at +0x360, +0x368, +0x370 and +0x378): the dword after the
// limit (+0x364, +0x36c, +0x374, +0x37c) when the staff of kind 5 / 6 / 4 / 7 (Things_CountStaffOfKind) outnumber the objects counted for
// definition field 0 (Things_CountByDef4ac(0, 0)) plus the limit (signed); else 0.
u32 __fastcall Staff_SurplusKind5(const u8 *object);
u32 __fastcall Staff_SurplusKind6(const u8 *object);
u32 __fastcall Staff_SurplusKind4(const u8 *object);
u32 __fastcall Staff_SurplusKind7(const u8 *object);

}  // extern "C"
