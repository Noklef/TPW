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

// 0x402a50, 0x402b10, 0x402b80, 0x402bf0, 0x402c60, 0x402cd0 (thiscall (i), this unused): &key record i of the class's schema.
const u8 *__fastcall BalanceHoardings_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceLevel_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceOnline_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceGraphics_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceResolution_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall BalanceSound_GetSchema(const void *, void *, u32 index);
}
