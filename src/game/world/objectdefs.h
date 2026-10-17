// The object definitions (loaded by ObjectDefs_Load 0x413c10 from .sam files): reimplementation
// of the lookups 0x412e90, 0x412ef0 and 0x412f50, the walk 0x412e60 / 0x412e70, the marked cells 0x413410 and the schema slots 0x412c90
// and 0x4143f0. The set holds up to 0x14a definition pointers from +0, their count at +0x528, a
// walk cursor at +0x72c and a one-entry cache, +0x730 (u16) the id last found and +0x734 its
// definition; a definition starts with its u16 id.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x412e90: the definition with id (the cached one when id is the cached id, whatever it
// holds; else the first in the list, which is then cached), or 0.
u8 *__fastcall ObjectDefs_Find(u8 *defs, void *, u16 id);

// 0x412ef0 / 0x412f50: the found definition's +0x390 / +0x410 part, or 0.
u8 *__fastcall ObjectDefs_FindAt390(u8 *defs, void *, u16 id);
u8 *__fastcall ObjectDefs_FindAt410(u8 *defs, void *, u16 id);

// 0x412e60: the first definition (+0, read whatever the count), the walk's cursor set to it.
u8 *__fastcall ObjectDefs_First(u8 *defs);

// 0x412e70: the definition after the walk's cursor (+0x72c), which moves on to it; 0 at the last.
u8 *__fastcall ObjectDefs_Next(u8 *defs);

// 0x4d3e00 (thiscall on a table of 0x20-byte records, count at +0x12c0, a lookup cache of the last id
// at +0x12c4 and its record at +0x12c8): walks the object definitions (0x7890a0, First and Next) and for
// each whose id (word at +0) has no record yet adds one: the id, the definition's dwords at +0x1b8, +0xe8
// and +0x16c, the byte +0x10 = (the dword at +0x180 is 0) and +0x14, +0x18, +0x1c = 0. A cache that
// matches the id with a null record adds one too.
void __fastcall ObjectControls_Collect(u8 *table);

// 0x4d3d10 / 0x4d3d80 (thiscall on the same table; id, quiet, pops 8): the record whose id word (+0) equals the low 16 bits of the id, the
// cached one first when the cache id matches; 0 for none (the original reports that to its empty debug printf unless quiet is not 0).
// The first also makes the record found the cache.
u8 *__fastcall ObjectControls_Find(u8 *table, void *, u32 id, u32 quiet);
u8 *__fastcall ObjectControls_FindUncached(u8 *table, void *, u32 id, u32 quiet);

// 0x4d3de0 (thiscall on the table): its 0x4b3 dwords cleared, then ObjectControls_Collect.
void __fastcall ObjectControls_Init(u8 *table);

// 0x4ca740 (cdecl): over the object definitions (First / Next) those whose dword at +0xc4 is above 0 are counted, and of those the ones whose control
// record (ObjectControls_FindUncached, quiet, in the game's table) has its dword at +0x18 above 0; the percentage (second * 100 / first), 0 when the
// first is 0.
s32 __cdecl ObjectDefs_PercentWithControls();

// 0x4ca8c0 (stdcall; id): the dword at +0x18 of the record ObjectControls_FindUncached finds for the id (quiet) in the game's table (at +0x1d943c of
// the object [0x80239c] points to); the record must exist.
u32 __stdcall ObjectControls_GetWord18(u32 id);

// 0x4e2740 (thiscall on an object): ObjectControls_Find (quiet) of the word at +0xe of the object in the game's table.
u8 *__fastcall Obj_FindControls(const u8 *object);

// The schemas of the .sam files (vtable slots, this unused): record index of 0x3c bytes.
// 0x412c90: an object definition's (0x744b30). 0x4143f0: a track type's (0x739ea8).
const u8 *__fastcall ObjectDef_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall TrackType_GetSchema(const void *, void *, u32 index);

// 0x59c320 (stdcall; id): the byte at +0x20 of the 56-byte record at 0x76e300 + 56 * id when its id (+4) matches; 0 for 0x15f and 0x160 and for
// ids that match no record (searched from 0x76e304 on, but a match found by the search gives the byte of the record at id's own slot: sic).
u8 __stdcall Table76e300_GetByte(u32 id);

// 0x59c390 (thiscall; id, pops 4): the record of the table at 0x76e300 for id (see Table76e300_GetByte: sic, the record at the id's own slot even
// when the search found the id elsewhere): the dword at 0x24 + 12 * (the record's dword at +8) of the object; 0 for no record.
u32 __fastcall Table76e300_GetObjectDword(const u8 *object, void *, u32 id);

// 0x59c410 (thiscall; id, pops 4): the same with the dword at 0x28 + 12 * (...) being not 0 (as AL; 0 for no record).
u8 __fastcall Table76e300_ObjectDwordSet(const u8 *object, void *, u32 id);

// The record of the table at 0x76e300 that the lookups above use (not a game function): see Table76e300_GetObjectDword.
u32 Table76e300_FindRecord(u32 id);

// 0x59c570 (thiscall; id, limit, pops 8): the record of the table at 0x76e300 for id (see Table76e300_FindRecord) gives an index (its dword at +8; 10 for
// no record); for 10 the result is 0, otherwise the dword at 0x2c + 12 * index of the object must not be 0 and the signed limit must be at least that
// (1 or 0, as AL).
u8 __fastcall Table76e300_LimitCheck(const u8 *object, void *, u32 id, s32 limit);

// 0x4e2b80 (thiscall): whether the dword at +0x9c of the object definition for the id at +0xe (ObjectDefs_Find, looked up again for each test) is 1, 2
// or 3.
u32 __fastcall Obj_DefKind123(const u8 *object);

// 0x59c610 (stdcall; id): the 0x38-byte records from 0x76e300 hold their id at +4; false for the ids 0x15f and 0x160 and for an id found nowhere (the table is searched up to
// 0x772fcc; a printed message is left out); otherwise whether the dword at +8 of the record at id * 0x38 is 1 (the record is that one even when the search found the id
// elsewhere, sic).
u8 __stdcall Record_IsActive(u32 id);

// 0x59c240: the dword at +0x24 of the object definition with the id (0x266 when there is none).
u32 __stdcall DefField24_59c240(u32 v0);

// 0x59c2b0: the dword at +0x28 of the object definition with the id (0x0 when there is none).
u32 __stdcall DefField28_59c2b0(u32 v0);

// 0x59c490: the dword at +0x2c of the object definition with the id (0x5 when there is none).
u32 __stdcall DefField2c_59c490(u32 v0);

// 0x59c500: the dword at +0x8 of the object definition with the id (0xa when there is none).
u32 __stdcall DefField8_59c500(u32 v0);
// 0x413410 (thiscall, this unused; a definition variable, pops 4; name unconfirmed): in the definition's cell grid (+0x18: columns, rows, cells of 8 bytes
// 20 to a row, a kind and a byte; scanned column by column with the row count read once) the first cell of kind 9 gives +0x494 / +0x498 (its column / row)
// and the byte +0x49c (its byte), and the first of kind 0xa +0x4a0 / +0x4a4 and +0x4a8 the same way (the kind 9 cell's when there is none); with no kind 9
// cell those are 0, 0, 1, 0, 0 and 0x10. Then +0x4b0 / +0x4b4 = +0x30 / +0x34, the u16 +0 = +0x10 and +0x4ac = +0x4c.
void __fastcall ObjectDef_FindMarkedCells(void *, void *, u8 **def);

// 0x412c20 (thiscall; returns the definition): the balance file at +4 gets vtable 0x6fd8a8 and is laid out (BalanceFile_Init); the runtime
// fields +0x494, +0x498, +0x4a0, +0x4a4, +0x4b0, +0x4b4, +0x4e0-+0x4f0 and +0x4f4-+0x500 are cleared.
u8 *__fastcall ObjectDef_Ctor(u8 *def);
}
