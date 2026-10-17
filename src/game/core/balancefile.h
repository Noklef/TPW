// The .sam-backed balance files (BalanceFile): a vtable whose slot 0 returns key record i of the class's schema (0x3c bytes: +0 type, +0x24 the
// integer minimum, +0x2c the float minimum, +0x34 a group's count; type 0xc ends it) and the values at +4, one 0xc-byte entry per key record
// (+0 the values, +4 their stride in dwords, +8 a pointer cleared with them). Reimplementation of 0x401250, 0x4013e0, the schema getters and the
// static destructors at 0x402a50-0x402cf0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4013e0 (thiscall; keep): the values set to their defaults. Key records are walked in groups: a group starts at a record (its count the +0x34
// of the first type-3 record from there when it has type 2, else 1; the starting record itself is passed over (sic)) and runs over the records
// after it up to and including one of type 1 or 3. Each value of a record (count of them, stride apart) becomes 0 for types 4, 5, 7 and 8, the
// record's +0x24 for type 6, its +0x2c for type 9; for types 10 and 11 (strings) each value is freed (Mem_GlobalFree) unless keep is 1 and
// cleared. The entry's +8 pointer, when set, is cleared for types 4-11. Types 0 and 2 inside a group were an assert.
void __fastcall BalanceFile_SetDefaults(u8 *self, void *, u32 keep);

// 0x401250 (thiscall; name unconfirmed): with values, BalanceFile_SetDefaults(0); the values block is freed (Mem_GlobalFree) and cleared.
void __fastcall BalanceFile_Release(u8 *self);

// 0x401280 (thiscall (group, name, &index, grouped, row), pops 20; name unconfirmed): finds a key by its section and name. Counting the key
// records up to type 0xc, the last record from there back of type 1 or 3 whose name (+4) is group is the section (none: 0); it must be of
// type 3 when grouped (with row below its +0x34 count, else 0 after an assert, left out) and of type 1 when not. From the record before it
// back to the first, stopping (0) at one of type 0 or 2, the first named name: index = its number, returns 1; none: 0.
u32 __fastcall BalanceFile_FindKey(u8 *self, void *, const char *group, const char *name, s32 *index, u32 grouped, s32 row);

// 0x402a50, 0x402b10, 0x402b80, 0x402bf0, 0x402c60, 0x402cd0 (thiscall (i), this unused): &key record i of the class's schema.
const u8 *__fastcall BalanceHoardings_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceLevel_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceOnline_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceGraphics_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceResolution_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceSound_GetSchema(const void *, void *, u32 index);

// 0x401030 (thiscall): the values entries laid out. Counting the key records up to type 0xc, a block of 0xc bytes for each is allocated
// (Mem_GlobalAlloc; none: nothing more) into +4. Walking the records with a running dword index from 1 into the values (vtable slot 1,
// BalanceFile_GetValues): a value record (types 4-11) takes the next dword, with the stride of its group and the group's pointer; a group start
// (type 2) counts its members up to the next record of type 0-3 (the count at that record's +0x34 sizes the group: its pointer is the values
// at index + count * members) and a group end (type 3) moves the index past the group's other rows; types 0-3 leave an empty entry and other
// types none. Then BalanceFile_SetDefaults(1).
void __fastcall BalanceFile_Init(u8 *self);

// 0x402a70, 0x402ac0, 0x402b30, 0x402ba0, 0x402c10, 0x402c80, 0x402cf0 (cdecl, at exit; names unconfirmed): the static balance files
// (gBalanceHoardingsA / B, gBalanceLevel, gBalanceOnline, gBalanceGraphics, gBalanceResolution, gBalanceSound) get their vtable back and are
// released (BalanceFile_Release).
void __cdecl BalanceHoardingsA_StaticDtor();
void __cdecl BalanceHoardingsB_StaticDtor();
void __cdecl BalanceLevel_StaticDtor();
void __cdecl BalanceOnline_StaticDtor();
void __cdecl BalanceGraphics_StaticDtor();
void __cdecl BalanceResolution_StaticDtor();
void __cdecl BalanceSound_StaticDtor();

// 0x412d40, 0x414410, 0x423dc0 (thiscall; names unconfirmed): the same for a balance file of the object definitions (vtable 0x6fd8a8), the
// track types (0x6fd8b0) and the graphics settings (0x6fd630).
void __fastcall ObjectDefBalance_Dtor(u8 *self);
void __fastcall TrackTypeBalance_Dtor(u8 *self);
void __fastcall BalanceGraphics_Dtor(u8 *self);

// 0x415e90 (thiscall (i), this unused): &key record i of the advisor scoring schema at 0x72e120.
const u8 *__fastcall AdvisorScoring_GetSchema(const void *, void *, u32 index);

// 0x415e80 (thiscall; name unconfirmed): the balance file at +0xc gets vtable 0x6fd958 back and is released (BalanceFile_Release).
void __fastcall AdvisorScoring_Dtor(u8 *self);

// 0x59bf10 (thiscall; name unconfirmed): the advisor scoring balance file itself (the one at +0xc, an unwind action of
// AdvisorScoring_Construct) gets its vtable 0x6fd958 back and is released.
void __fastcall AdvisorScoringFile_Dtor(u8 *self);

// 0x415e70 (cdecl, at exit; name unconfirmed): AdvisorScoring_Dtor of gAdvisorScoring (0xfb3540).
void __cdecl AdvisorScoring_StaticDtor();
}
