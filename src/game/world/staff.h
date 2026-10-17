// The staff base class's small methods (the thing's pay grade +0x1e4, happiness +0x1f8,
// energy +0x1fc, name +0x1a0, mState +0x19c, mRestArea +0x208, mPatrolRegionBL +0x20a and TR
// +0x20c, two map ids): reimplementation of 0x506650-0x5068f0 and 0x506e50-0x506ed0, the staff
// pool's 0x507bd0 and 0x5087b0, the staff check and list heads 0x481f80-0x482020, and the
// guard's state checks 0x4d6330 / 0x4d6350, the training 0x505a10 (progress byte +0x1e8) and the count by type 0x508000. The staff pool (0x507850) is 0x20 entries of 20
// bytes (mType +0, mName +4, u8 mPayGrade +8, mSubType +9, mValid +0xa, mOnPointer +0xb).
// A patrol region of (1, 0x4000), the whole map, is none.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x506680 (thiscall): whether the energy (+0x1fc), __ftol'd and cut to its low byte, is
// below the rest level ([0x785324], signed).
u32 __fastcall Staff_IsTooTired(const u8 *staff);

// 0x5066a0 (thiscall): energy less (6 - grade) * 0.012 and happiness less (6 - grade) *
// 0.005, each clamped to [0, 100] (above 100 gives 100; below 0, or NaN, gives 0).
void __fastcall Staff_TireA(u8 *staff);

// 0x506760 (thiscall): as Staff_TireA by 0.025f and 0.01; the happiness drain is stored to a
// float before it is added.
void __fastcall Staff_TireB(u8 *staff);

// 0x506830 (thiscall): wcsncpy of 32 units of name to +0x1a0, and the unit after them
// (+0x1e0) zeroed.
void __fastcall Staff_SetName(u8 *staff, void *, const u16 *name);

// 0x506860 (thiscall): the name, +0x1a0.
u16 *__fastcall Staff_GetName(u8 *staff);

// 0x481f80 (thiscall on a thing): whether its type (byte +2) is a staff one, 4 to 8.
u32 __fastcall Thing_IsStaff(const u8 *thing);

// 0x481fa0, 0x481fc0, 0x481fe0, 0x482000, 0x482020 (thiscall on the world; &handle): the
// handle at the head of the handyman, mechanic, entertainer, researcher or guard list (u16
// world +0x1da73c, +0x1da73e, +0x1da740, +0x1da742, +0x1da744; each thing's next at +0x210).
// Return the pointer.
u16 *__fastcall World_FirstHandyman(const u8 *world, void *, u16 *out);
u16 *__fastcall World_FirstMechanic(const u8 *world, void *, u16 *out);
u16 *__fastcall World_FirstEntertainer(const u8 *world, void *, u16 *out);
u16 *__fastcall World_FirstResearcher(const u8 *world, void *, u16 *out);
u16 *__fastcall World_FirstGuard(const u8 *world, void *, u16 *out);

// 0x519650, 0x5197c0, 0x519930, 0x519c10, 0x519aa0 (thiscall on the world; thing): the thing
// pushed on the head of the handyman, mechanic, entertainer, researcher or guard list (+0x1da73c,
// 73e, 740, 742, 744; its +0x210 the old head, unless that was 0). 0x519d80: the same for the
// object list at +0x1da746, next at the thing's +0xc.
void __fastcall World_AddHandyman(u8 *world, void *, u8 *thing);
void __fastcall World_AddMechanic(u8 *world, void *, u8 *thing);
void __fastcall World_AddEntertainer(u8 *world, void *, u8 *thing);
void __fastcall World_AddResearcher(u8 *world, void *, u8 *thing);
void __fastcall World_AddGuard(u8 *world, void *, u8 *thing);
void __fastcall World_AddObject(u8 *world, void *, u8 *thing);

// 0x4d6330 / 0x4d6350 (thiscall on a guard): whether its state (+0x19c) is 0 or 1 (the
// nearest guard search takes only those), and whether it is 0x12.
u32 __fastcall Guard_IsFree(const u8 *guard);
// 0x500f00 (thiscall; out, pops 4): the thing of the list that starts at the handle the word at +0x1da740 of the game object [0x80239c] holds (chained through
// the word at +0x210) that is nearest the object, by the squared distance between their map columns and rows (bytes +5 and +7; the first of equals wins);
// its handle (0: none) is stored through out. Returns out.
u16 *__fastcall Staff_NearestInList1da740(const u8 *object, void *, u16 *out);
// 0x5099c0 (thiscall; out, unused, pops 8): the same over the list at +0x1da744 but only things Guard_IsFree holds for (the nearest guard search).
u16 *__fastcall Guard_NearestFree(const u8 *object, void *, u16 *out, u32 unused);
u32 __fastcall Guard_IsState12(const u8 *guard);

// 0x506650 (thiscall; area): whether mState is 3 and mRestArea is the area (its low 16 bits).
u32 __fastcall Staff_IsState3At(const u8 *staff, void *, u32 area);

// 0x5068f0 (cdecl; type, grade): the pay grade's wage ([0x78532c + (grade & 0xff) * 16]) times
// the type's factor ([0x785380 + type * 4]).
s32 __cdecl Staff_WageFor(u32 type, u32 grade);

// 0x506870 (thiscall): the staff member's wage: the factor of the type its model byte (+2) names (4 to 8 are the types 1, 0, 2, 3, 4; any other model is the type 5, the
// debug message of the original is gone) times the wage of the pay grade in the dword at +0x1e4 (16 bytes each from 0x78532c).
s32 __fastcall Staff_GetWage(const u8 *staff);

// 0x506e50 (bl, tr) / 0x506e70 (&bl, &tr) / 0x506e90: the patrol region set, read (bl, then
// tr), or set to none; 0x506eb0: whether it is not none.
void __fastcall Staff_SetPatrolRegion(u8 *staff, void *, u32 bl, u32 tr);
void __fastcall Staff_GetPatrolRegion(const u8 *staff, void *, u16 *bl, u16 *tr);
void __fastcall Staff_ClearPatrolRegion(u8 *staff);
u32 __fastcall Staff_HasPatrolRegion(const u8 *staff);

// 0x506ed0 (pos, its low 16 bits): whether the map id's column and row are within the patrol
// region's (a map id less 1: column & 0x7f, row >> 7; an id of 0 wraps to column 0x7f, a
// row past the map).
u32 __fastcall Staff_InPatrolRegion(const u8 *staff, void *, u32 pos);

// 0x507bd0 (index): the pool's entry when index < 0x20 (unsigned) and it is mValid, else null.
u8 *__fastcall StaffPool_ValidEntry(u8 *pool, void *, u32 index);

// 0x5087b0: the index of the pool's first entry not mValid, 0x20 when all are.
u32 __fastcall StaffPool_FirstInvalid(const u8 *pool);

// 0x50c620: the staff training (serialized at 0x50c640: 5 x u32 mBudget, u8 mHaveEverTrained)
// zeroed. Returns it.
u8 *__fastcall StaffTraining_Ctor(u8 *training);

// 0x5077f0 (thiscall): Staff_WageFor of the dword at +0 and the byte at +8.
s32 __fastcall Staff_WageForRecord(const u8 *record);

// 0x503980 (thiscall on the table; index, pops 4): 0 when Global7852f8_Times78548c is 0, else the dword at table + (index + 0x141) * 16 times (Things_SumChain * 100 / that
// total, unsigned), divided by 100 (unsigned).
u32 __fastcall Table_ScaledBySum(u8 *table, void *, s32 index);

// 0x505a10 (thiscall; amount, pops 4; name unconfirmed): training paid for. Nothing at grade 4 (a message is printed, left out). Otherwise the money record (the
// thing of the world's handle at +0x1da726) is debited the amount (Money_Debit), the park's (handle +0x1da720) dword at +0x1fa40 gains it, and the amount divided by
// the cost of a point for the type (byte +2) and grade (+0x1e4): handyman 0x785430 + grade * 8, mechanic 0x7853f4 + grade * 12, entertainer 0x7853a4 + grade * 16,
// researcher 0x78549c + grade * 8, guard 0x785460 + grade * 12; another type divides by the amount itself (after a message). That, at most 100, is added to the
// progress byte at +0x1e8; reaching 100 raises the grade, keeps the progress over 100 and sets the happiness (+0x1f8) to 100.
void __fastcall Staff_Train(u8 *staff, void *, s32 amount);

// 0x508000 (thiscall; name unconfirmed): the staff of each type counted over the world's things (World_FirstThing / World_NextThing on [0x80239c]): the dwords
// at +0x280 to +0x290 and the flag bytes +0x299 to +0x29d are zeroed first; a mechanic (type 5, byte +2) counts at +0x280, a handyman (4) at +0x284, an
// entertainer (6) at +0x288, a researcher (7) at +0x28c and a guard (8) at +0x290, and a count reaching its limit (0x7852e8, 0x7852ec, 0x7852f0, 0x7852f4,
// 0x7852f8 in the same order, signed) sets the flag (+0x299, +0x29a, +0x29b, +0x29c, +0x29d).
void __fastcall Staff_CountTypes(u8 *counts);
}
