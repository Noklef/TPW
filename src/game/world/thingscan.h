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

// 0x4c8430 (stdcall; three byte outputs): over the things of kind 1 whose analyser cell is plain (AnalyserCell_TypePlain), the three values of the dword at +0x30
// (0 to 22; not checked) that occur most, counting from value 3 up and starting from {0, 1, 2} (the count of 0 forced to 0): a later value replaces the entry it beats by the
// most (a strictly larger gain). Each output is the entry, or 0x17 when its count is 0.
void __stdcall Things_TopThreeValues(u8 *first, u8 *second, u8 *third);

// 0x4c8d30 (stdcall; kind, cell id, radius, first only): the things of the kind (the byte at +2) in the lists of the cells within the radius (clamped to 0 to 0x7f)
// of the cell: the list head is the word at -0x20 of each 0x44-byte cell, the links the words at +0xa of each thing. Returns 1 at the first one when first only is set, else
// the count.
s32 __stdcall Things_CountOfKindAround(u32 kind, u32 cell, s32 radius, s32 firstOnly);

// 0x4c8eb0 (stdcall; kind, cell id, radius): Things_CountOfKindAround with first only set (1 when there is one, else 0).
s32 __stdcall Things_AnyOfKindAround(u32 kind, u32 cell, s32 radius);

// 0x4d9280 (thiscall on the game object; thing handle, pops 4): takes the thing out of the doubly linked chain through its words at +8 (previous) and +0xa (next): the head
// (the word at +0x24 of the game object) becomes its next when it has no previous, otherwise the previous's next is its next; the next's previous is its previous. Both of its
// own links are cleared.
void __fastcall Things_UnlinkFromChain(u8 *game, void *, u32 handle);

// 0x4c8ed0 (thiscall on the world block; out word, type, cell id, radius, pops 16; the radius check is an assert left out): the ids of the cells within the radius
// (clamped to the map, x outer, y inner) whose dword at +0x2c is not 0 and whose word at +0x30 is the low half of type or 0 are listed at world + 0x1f020; *out is the nearest
// of them to the cell by x distance + y distance (the first of equals), 0 for none. Returns out.
u16 *__fastcall World_NearestCellWithType(u8 *world, void *, u16 *out, u32 type, u32 cell, s32 radius);
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
// 0x59ef10: the same for the staff of kind 8 with the limit at +0x380 and the dword after it at +0x384.
u32 __fastcall Staff_SurplusKind8(const u8 *object);

// 0x506910 (thiscall on a thing; out word, pops 4; the "Looking for rest area" print is left out): the handle of the thing in the game's list (the head word at +0x1da746 of
// [0x80239c], each thing's next at +0xc) whose flags at +0x32 have bit 2 and whose cell is nearest to the thing's (squared distance of the column and row bytes at +5 and +7; the
// first of equal ones wins); 0 when none.
void __fastcall Things_NearestRestArea(const u8 *thing, void *, u16 *out);

// 0x501020 (thiscall on a thing; out word, pops 4): the same for the things with bit 0x80 in the flags whose ride (Rides_FindById of the dword at +0x24) has no element 0
// (Rides_GetElement), and only within a squared distance of 4.
void __fastcall Things_NearestFlag80Close(const u8 *thing, void *, u16 *out);

// 0x500dc0 (thiscall on a moving person; out word, radius, pops 8; name unconfirmed; an assert on the thing table is left out): over the same list, each
// thing with bit 0x40 in the flags closer than the radius (squared, unsigned, as both squares wrap) and than the best so far, whose cell (the word at
// +0x36) the person can be sent to (Sprite_SetDestinationCell, which is left set), becomes the best; its handle goes to out (0 for none). Returns out.
u16 *__fastcall Things_HeadForNearestFlag40(u8 *sprite, void *, u16 *out, u32 radius);

// 0x4d99e0 (thiscall on a 0x44-byte map cell; out word, pops 4; name unconfirmed; an assert on the thing table is left out): the first thing of kind
// 3 (an object; the byte at +2) in the cell's chain (the head word at +0x24, the links the words at +0xa of each); its handle to out, 0 for none. Returns out.
u16 *__fastcall Cell_FirstObject(const u8 *cell, void *, u16 *out);

// 0x527d60 (cdecl; out word, &x, &y; name unconfirmed; an assert on the thing table is left out): the object on a map cell. Outside the map
// (Map_InBounds) or for a cell (of [0x7cf83c], id (y << 7) + x + 1 cut to 16 bits) not of type 4, 9 or 0xa: 0 to out, x and y left. Else from
// the cell, or its parent (the word at +0x10) when that is another, Cell_FirstObject's handle goes to out, and x and y become the cell of that
// object (its word at +0x34) or, with none, of the cell looked in: (id - 1) & 0x7f and (id - 1) >> 7. Returns out.
u16 *__cdecl Map_ObjectAtCell(u16 *out, s32 *x, s32 *y);

// 0x527e80 (cdecl; out word, x, y; name unconfirmed): on the entrance's row (the dword at 0x7855c0) from a cell before its first column (0x7855bc)
// to one after its last (0x7855c4; signed), the word at +0x1da732 of [0x7cf83c] (the entrance's thing); elsewhere Map_ObjectAtCell. Returns out.
u16 *__cdecl Map_ObjectOrEntranceAt(u16 *out, s32 x, s32 y);

// 0x4824d0 (cdecl; category, previous; name unconfirmed): the next thing of a category (one bit) after the previous one (from the first when 0), each by
// its own finder: 1 a staff member (Thing_IsStaff) in the in-use list (World_FirstThing / World_NextThing); 2, 4, 8, 0x10, 0x20 the next in a staff
// list: the head (the handymen's word world +0x1da73c read inline, World_FirstMechanic, World_FirstGuard, World_FirstEntertainer,
// World_FirstResearcher on the world 0x7cf83c) or the previous one's link word at +0x210, its thing by Thing_ByHandle (4: read from the thing table
// inline); 0x40 a thing of kind 1 in the in-use list (Thing_ByHandle); 0x80, 0x100, 0x200, 0x400 Things_FindNextByDef4ac with 0, 2, 1, 3; 0x800
// Things_FindNextShopNoFlags; 0x1000 Things_FindNextShopFlag1; 0x2000 Things_FindNextWithDefWord; 0x4000 Things_FindNextWithDefWordUpgrade2; 0x8000
// Things_FindNextKind3Flag2. Anything else: 0.
u8 *__cdecl Things_FindNextOfCategory(s32 category, const u8 *previous);

// 0x482da0 (cdecl; category, previous; name unconfirmed): the same backwards (World_PrevThing; from the oldest thing when previous is 0): 1 a staff
// member, from the oldest found by walking the whole list forwards (Thing_ByHandle, Thing_GetHandle of the last: sic, of a null thing when there are
// none); 2, 4, 8, 0x10, 0x20 and 0x40 all a thing of kind 1 (from Things_LastHandle, the thing table read inline); 0x80-0x400 Things_FindPrevByDef4ac
// with 0, 2, 1, 3; 0x800 / 0x1000 inline: an object (kind 3) whose definition word +0x284 is 0 (Object_DefWord284) and dword +0x4ac 3
// (Object_GetDef) and whose byte +0x32 has neither bit 0 nor 1 (0x800, the thing table read inline) / bit 0 (0x1000, Thing_ByHandle); 0x2000
// Things_FindPrevWithDefWord; 0x4000 Things_FindPrevWithDefWordUpgrade2; 0x8000 Things_FindPrevKind3Flag2. Anything else: 0.
u8 *__cdecl Things_FindPrevOfCategory(s32 category, const u8 *previous);

// 0x483740 (cdecl; name unconfirmed): Things_FindNextOfCategory after previous; past the last, again from the start (previous 0).
u8 *__cdecl Things_FindNextOfCategoryWrap(s32 category, const u8 *previous);

// 0x483770 (cdecl; name unconfirmed): the same for Things_FindPrevOfCategory: before previous; past the first, again from the end (previous 0).
u8 *__cdecl Things_FindPrevOfCategoryWrap(s32 category, const u8 *previous);

// 0x4b3340 (cdecl; name unconfirmed): the thing browser's (an MFC dialog at 0x7cc304) step: for its choice (the dword at 0x7cc30c) 1..6, the
// category 2, 4, 8, 0x10, 0x20 or 0x40; the thing at 0x7cc308 = Things_FindNextOfCategory after it, else (past the last) from the start (as
// Things_FindNextOfCategoryWrap, the first result stored before the second call); another choice: 0.
void __cdecl ThingBrowser_Step();

// 0x4cceb0 (thiscall on the park analyser, which it only hands on; name unconfirmed; the analyser's 0x4c7a49 records it in its sixteenth ring):
// the sum of capped counts: the things of kind 1 on a plain analyser cell (AnalyserCell_TypePlain; at most 1000) * 20 / 1000;
// Things_CountByDef4ac(0, 0) * 3 / 2 (at most 20); (1, 0) * 2, (2, 0) * 2, (3, 0) and (0, 2) (at most 10 each); and the staff of kinds 5, 4, 6,
// 7 and 8 (Things_CountStaffOfKind; at most 4 each). Each capped count is counted again when under its cap (the same answer).
s32 __fastcall Things_FacilityScore(void *analyser);

// 0x4caae0 (thiscall on an object it does not read, from 0x519510; category, &out; pops 8; name unconfirmed): of the objects (kind 3) whose
// definition's category (+0x164) is the one asked for and whose age (Object_AgeDays) is over 30 days, the one with the most walk-aways per 100
// customers: 100 times the CHistory sum (CircBufU32_30_Sum, newest 30) of the walk-aways' ring (+0x230) over that of the customers' (+0x1a8, summed
// twice), __ftol's low halves, unsigned, 32-bit; objects whose customers' sum is 0 are passed over. Its handle goes to *out (0 for none) and its
// percentage (the larger wins, signed; the first on a tie) is returned, 0 for none. The debug prints of each ring's count are left out.
s32 __fastcall Things_MostWalkAwaysOf(void *self, void *, u32 category, u16 *out);

// 0x4cb300 (thiscall as 0x4caae0; &out; pops 4; name unconfirmed): the same for the categories 1, 5 and 6.
s32 __fastcall Things_MostWalkAwaysOf156(void *self, void *, u16 *out);

// 0x4cadf0 / 0x4cb640 (thiscall as 0x4caae0 / 0x4cb300; names unconfirmed): the same with the fewest walk-aways per 100 customers (the
// smaller wins, signed; the first on a tie).
s32 __fastcall Things_FewestWalkAwaysOf(void *self, void *, u32 category, u16 *out);
s32 __fastcall Things_FewestWalkAwaysOf156(void *self, void *, u16 *out);

// 0x4cbba0 / 0x4cc090 (thiscall as 0x4cb300; names unconfirmed): the most / the fewest for the objects whose definition (ObjectDefs_Find on
// gObjectDefs 0x7890a0 by the id +0xe) has 2 at +0x4ac.
s32 __fastcall Things_MostWalkAwaysDef4ac2(void *self, void *, u16 *out);
s32 __fastcall Things_FewestWalkAwaysDef4ac2(void *self, void *, u16 *out);

// 0x4cb100 / 0x4cb200 (thiscall as 0x4caae0; names unconfirmed): of the objects older than 30 days with that category, the happiest /
// unhappiest (Object_HistoryAverage, signed; the first on a tie); its handle to out (0 for none); returns its average (0 for none).
s32 __fastcall Things_HappiestOf(void *self, void *, u32 category, u16 *out);
s32 __fastcall Things_UnhappiestOf(void *self, void *, u32 category, u16 *out);

// 0x4cb980 / 0x4cba90 (thiscall as 0x4cb300; names unconfirmed): the same for the categories 1, 5 and 6.
s32 __fastcall Things_HappiestOf156(void *self, void *, u16 *out);
s32 __fastcall Things_UnhappiestOf156(void *self, void *, u16 *out);

// 0x4cbec0 (thiscall as 0x4cb300; name unconfirmed): the happiest of the objects whose definition has 2 at +0x4ac (as 0x4cbba0's) and
// that had customers (as the walk-aways'). The debug prints of the size check are left out.
s32 __fastcall Things_HappiestDef4ac2(void *self, void *, u16 *out);

// 0x4c7bb0 (thiscall, pops 4; the object and argument unused; name unconfirmed): the float [0x70031c] while the world's dword +0x1da710 is set
// (Get1da710_51a280); else the average over the things of kind 1 on a plain analyser cell (AnalyserCell_TypePlain) of the low byte of
// their float +0x19c cut to an integer (x87: each sum by FIADD, cut again; the average FIDIV at the game's precision), [0x70031c] when there
// are none. The debug prints of the slot check are left out.
double __fastcall Things_AveragePersonStat19c(void *self, void *, u32 unused);

// 0x4c7cc0 (as 0x4c7bb0, without the +0x1da710 check; name unconfirmed): the same over the staff (kinds 4 to 8) of their float +0x1f8.
double __fastcall Things_AverageStaffStat1f8(void *self, void *, u32 unused);

// 0x4cc810 (stdcall (out, definition), pops 8; name unconfirmed): over the things of kind 3 whose definition is of kind 3
// (Object_DefIsKind3), of the definition id asked for (the low words) unless that is 0, the one whose coaster link (Object_LinkParams,
// which must be found) has the most flagged cars (+0x108, unsigned; the first of a tie): its handle to out (0 beforehand), the count
// returned, 0 when none. The debug prints of the slot check are left out.
u32 __stdcall Things_CoasterMostFlaggedCars(u16 *out, u32 def);

// 0x4cc920 (as 0x4cc810; name unconfirmed): the same for the tallest, the link's height +0x110 a float: one taken when the best so far
// (the height cut to an integer, its low dword, unsigned) is below it or it is a NaN (x87), so a later 5.5 beats a 5.7 cut to 5, sic.
u32 __stdcall Things_TallestCoaster(u16 *out, u32 def);

// 0x4c7fa0 (thiscall on the analyser (maxAge), pops 4; name unconfirmed): the count of the things of kind 1 on a plain analyser cell
// (AnalyserCell_TypePlain) the analyser keeps at +0x1f0b8 (+0 the game time [0x80239c] +0x1da70c it was taken, +4 the count, byte +8 set):
// kept while +8 is set and it is no more than maxAge quarters old (Game1da70c_MinusAt0Quarters, unsigned); else counted again, +8 set and
// the time stamped (Out_SetFromGame1da70c). Returns +4. The debug prints of the slot check are left out.
u32 __fastcall ParkAnalyser_CountKind1OnPlainCell(u8 *analyser, void *, u32 maxAge);

// 0x4c8070 (as 0x4c7fa0, at +0x1f0c4; name unconfirmed): the count of the objects (kind 3) whose definition (ObjectDefs_Find) has 0 at
// +0x4ac.
u32 __fastcall ParkAnalyser_CountDef4ac0(u8 *analyser, void *, u32 maxAge);

// 0x4c9340 / 0x4c93f0 (thiscall, the analyser unused (least), pops 4; names unconfirmed): how many things of kind 1 on a plain analyser
// cell (AnalyserCell_TypePlain) have the low byte of their float +0x1a8 / +0x1a4 cut to an integer above least's low byte (unsigned).
// The debug prints of the slot check are left out.
s32 __fastcall Things_CountPeopleStat1a8Above(void *self, void *, u32 least);
s32 __fastcall Things_CountPeopleStat1a4Above(void *self, void *, u32 least);

// 0x4c9670 / 0x4c9750 (as 0x4c9340; names unconfirmed): the percentage (count * 100 / all, signed) of those things whose float +0x19c's
// low byte (as 0x4c9340's) is above / below bound (signed), 0 when there are none.
s32 __fastcall Things_PercentPeopleStat19cAbove(void *self, void *, s32 bound);
s32 __fastcall Things_PercentPeopleStat19cBelow(void *self, void *, s32 bound);
}  // extern "C"
