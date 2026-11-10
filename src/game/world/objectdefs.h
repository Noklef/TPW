// The object definitions (loaded by ObjectDefs_Load 0x413c10 from .sam files): reimplementation
// of the lookups 0x412e90, 0x412ef0 and 0x412f50, the walk 0x412e60 / 0x412e70, the marked cells 0x413410 and the schema slots 0x412c90
// and 0x4143f0. The set holds up to 0x14a definition pointers from +0, their count at +0x528, a
// walk cursor at +0x72c and a one-entry cache, +0x730 (u16) the id last found and +0x734 its
// definition; a definition starts with its u16 id.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

struct Stream;

// ObjectDefs_Load's and ObjectDef_LoadTrackTypes' calls by address (the bench's recorders).
void ObjectDefs_SetLoadCalls(const AddrCall *table, u32 count);

extern "C" {
// 0x413c10 (thiscall; entry, opener, source, pops 0xc; 1 or 0): one definition loaded. The folder source +0x18 taken into a local String, its end cut when it ends with the String 0x7898c8 (Names_Equal gives 1; then a
// backslash ensured). At the count +0x528 = 0x14a nothing more, 0. A new definition (Mem_Alloc 0x50c, ObjectDef_Ctor; none: a null entry, written through, sic) goes on the list; +0x28c the folder's text, +0x390 source +0xc's name
// widened (L"%S"). The .sam files read into its balance file (+4; each opened by a Class6fdaa4 open into the stream, named "<the handle's folder>\\<name>"; a failed read after its open: GameError_Set (3, 0, the name) and 0, sic but the
// first: no error): the entry's own (keep 0), in mode 1 the entry's online one, the source's own, in mode 1 its Online_ one or in mode 2 its Easy_ one. All parts there (Class6fdaa4_HasAllParts): +0x4f0 = Sound_Query (the folder, 0x74787c).
// A .emt: ObjectDef_LoadTrackTypes. ObjectDef_FindMarkedCells. The language's text: +0x504 bit 0, ObjectDef_LoadText. The stream closed. +0x410 the full name (0x40). Ride_LoadResource by +0x38 (flags 0x50c00, with +0x84 0x450c00; +0x28 and +0x2c
// its size) or without (0xb24a9 / 0x4b24a9; none); +0x490 its result. On one without +0x38 with a result and kind (+0x9c) 3: a balance file (vtable 0x6fd8b0) on the stack read from the source's coaster .sam (a failed read: GameError_Set, 0), CoasterType_Load
// (+0x490, the folder, it). +0x508 = no result; with a result +0x4b8 = +0x264 and that many of the layers loaded (the name at +0x244 + 4 i, flags 0xc0, id + i + 1, one) into +0x4bc + 4 i.
u32 __fastcall ObjectDefs_Load(u8 *defs, void *, u8 *entry, u8 *opener, u8 *source);
// 0x413930 (thiscall (folders), pops 4; name invented, unconfirmed): each source of the list at folders+0x2c searched for the wild card at 0x78a2f0 (slots 4, 8): a real directory or a wad (DirEntry_IsRealDir, DirEntry_IsWad) makes the path folders+0x18 + its name and a 0x38 byte
// object (0x41dc80). No object or a failed load ends the search, 0. The object must open its own file (0x41f170) and, in mode 2, its easy file (0x41f190), else it is deleted (slot 0 with 1). A name there already (0x41ede0) deletes it too, but not in mode 2 (sic: kept, not added).
// Added (0x41eaa0) and loaded (ObjectDefs_Load). No list: 0.
u32 __fastcall ObjectDefs_LoadFolders(u8 *defs, void *, u8 *folders);

// 0x414420 (thiscall on a definition; handle, pops 4; name invented, unconfirmed): the handle's files of the pattern 0x788f40 (slot 1 first, slot 2 next, a result >= 0 going on; the record's name its +0x20): each opened (slot 3) and, opened, its name
// less the suffix String 0x789010 (String_StripSuffix) gives its last character's number (atol) i: +0x4dc + 4 i = TrackTypeSlot_ReadIntoFree (the stream). The stream closed after each.
void __fastcall ObjectDef_LoadTrackTypes(u8 *def, void *, u8 *handle);


// 0x413600 (thiscall; stream, pops 4; name unconfirmed): a definition's text file (UTF-16, a BOM skipped): its four text fields +0x4f4
// first point at L"MISSING" (0x6fd880). The stream's size (vtable +0x14) must be even (else nothing more, after a debug print, compiled
// out); that many bytes are read (+8) into a block (Mem_GlobalAlloc, 2 more for the terminator). Nothing (sic: the block kept) when
// none came; else, from the start, while there is a line: its key must be one of the null-ended table 0x7487f8 (NAME, SIGNA, SIGNB;
// WText_WordEquals) or the reading stops; the line after it (past a line break of one or two characters, CR LF or LF CR, and past
// blank lines of spaces and tabs through WText_NextLine) is copied (Mem_GlobalAlloc) into the field the key's index picks, and the
// line after that is the next key's. The block is freed at the end.
void __fastcall ObjectDef_LoadText(u8 *def, void *, Stream *stream);

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

// 0x4d3aa0 (thiscall on the table; stream, save, version, unused): its 0x96 records of 0x20 bytes (mObjectControls[i]), the count at +0x12c0
// and the word at +0x12c4 (mPreviousSearchKey) saved or loaded as a serializer does (Save_Field); a load that gets them all clears the word just
// loaded (sic) and the dword at +0x12c8.
u32 __fastcall ObjectControls_Serialize(u8 *table, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4ca740 (cdecl): over the object definitions (First / Next) those whose dword at +0xc4 is above 0 are counted, and of those the ones whose control
// record (ObjectControls_FindUncached, quiet, in the game's table) has its dword at +0x18 above 0; the percentage (second * 100 / first), 0 when the
// first is 0.
s32 __cdecl ObjectDefs_PercentWithControls();

// The counts over the object definitions (First / Next on 0x7890a0) by their map kind (+0x4ac) and their control records
// (ObjectControls_FindUncached, quiet, in the game's table: at +0x1d943c of the object [0x80239c] points to; the record must exist). Names
// unconfirmed.
// 0x4ca4c0 (stdcall; an argument, unused): those of kind 0 whose record's dword +0x14 is above 0 (signed).
s32 __stdcall ObjectDefs_CountKind0WithControls14(u32 unused);
// 0x4ca510 / 0x4ca570 / 0x4ca5d0 (stdcall; an argument, unused): those of kind 1 / 2 / 3 whose record's byte +0x10 is set.
s32 __stdcall ObjectDefs_CountKind1WithControls10(u32 unused);
s32 __stdcall ObjectDefs_CountKind2WithControls10(u32 unused);
s32 __stdcall ObjectDefs_CountKind3WithControls10(u32 unused);
// 0x4ca450 (stdcall; flag): those of kind 0 whose record's byte +0x10 is set and whose dword +0xc4 is 0 or, with the flag's low byte set, any.
s32 __stdcall ObjectDefs_CountKind0WithControls10(u32 flag);
// 0x4ca7b0 (stdcall; kind, flag): those of the kind whose record's dword +0x18 is above 0 (signed) and whose dword +0xc4 is 0 or, with the
// flag's low byte set, any.
s32 __stdcall ObjectDefs_CountKindWithControls18(u32 kind, u32 flag);

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

// 0x412cb0 (thiscall; name invented, unconfirmed): a definition's four effects (+0x4e0-+0x4ec, each set one ParticleEffect_Clear118) and
// its sound category (+0x4f0, when set SoundCategory_Release) let go, then its balance file (+4) destroyed as ObjectDefBalance_Dtor does.
void __fastcall ObjectDef_Dtor(u8 *def);

// 0x4145f0 (thiscall; name invented, unconfirmed): from the walk index +0x72c set to 0, each definition from the first (+0) has its
// effects and sound category let go as ObjectDef_Dtor does; the index steps on (signed, below the count +0x528 less 1) to the next
// entry, stopping at a null one.
void __fastcall ObjectDefs_ReleaseEffects(u8 *defs);

// 0x412d50 (thiscall; name invented, unconfirmed): ObjectDefs_ReleaseEffects (inlined); then for each entry below the count +0x528 (signed,
// re-read each step) the definition's four texts (+0x4f4) freed (Mem_GlobalFree) and cleared unless null or the shared "MISSING STRING"
// (0x6fd880), the definition destroyed (ObjectDef_Dtor) and freed (Mem_Free) unless null, and the entry cleared; the count, the word +0x730
// and the dword +0x734 cleared.
void __fastcall ObjectDefs_Free(u8 *defs);

// 0x413140 (thiscall, a stack argument unused, pops 4; name invented, unconfirmed): ObjectDefs_ReleaseEffects; each entry below the count +0x528 (signed, re-read each step): its four buffers freed (Obj_FreeFourBuffers, even null),
// the definition destroyed and freed unless null, the entry cleared; the count, the word +0x730 and the dword +0x734 cleared. ObjectDefs_LoadFolders of the folders named 0x7898e0 and 0x788e60 in the list [0x786b9c] (NamedList_Find answering 1, else
// that one passed over); the vector of fonts [0x789c1c] emptied (RemoveFontResourceA, File_Delete of +8; then slot 3 (0) of each); the same for 0x78a2a0, 0x789d00, 0x789910. A load's 0 gives 0; else 1.
u32 __fastcall ObjectDefs_ReloadAll(u8 *defs, void *, u32 unused);

// 0x412a90 (cdecl; name invented, unconfirmed): ObjectDefs_Free of the definitions table 0x7890a0 (inlined).
void __cdecl ObjectDefs_FreeAll();

// The object texts (names unconfirmed): the table 0x7488d8 .. 0x749850 of 0xc-byte rows (an object id, its name's text id, its suffix's;
// an id 0 ends it) whose texts the text formatter (TextFormat, with the text table at 0x78bacc) writes. The object set's +0x52c holds
// three buffers of 0x40 characters for them: a name, a suffix and the two joined.

// 0x4146b0 (thiscall on the buffers; id, out, size, pops 12; returns 1 in AL): the name (0x40 chars; no row: an empty format, the bss
// string 0x78a374, through swprintf) and the suffix (+0x80, the same; the row looked up again); a name ending in '-' loses it and is joined
// to the suffix without a separator (L"%s%s"), else with a space when the suffix is not empty (L"%s%s%s", the space or the empty bss string
// 0x786de4); the full name (+0x100) is copied to out (wcsncpy, size capped at 0x80 as a signed number).
u8 __fastcall ObjectText_FullName(u16 *self, void *, u32 id, u16 *out, s32 size);

// 0x4147f0 (thiscall, this unused; id, out, size, trim, pops 16; AL): no row: 0; else the name formatted into out (size chars) and, when the
// trim byte is 1, a last '-' cut (a non-empty text ending in one); 1.
u8 __fastcall ObjectText_Name(void *, void *, u32 id, u16 *out, u32 size, u32 trim);

// 0x414870 (thiscall, this unused; id, out, size, pops 12; AL): no row: 0; else the suffix formatted into out; 1.
u8 __fastcall ObjectText_Suffix(void *, void *, u32 id, u16 *out, u32 size);

// The object set's names for an id (thiscall (id, out, size), pops 12; EAX incidental, the callers ignore it; names unconfirmed); each
// looks the definition up with the id's low word (ObjectDefs_Find):
// 0x412fb0: the definition's +0x4f4 text (wcsncpy, size chars) when it has one that is not L"MISSING" (0x6fd880), else ObjectText_FullName
// on the buffers at +0x52c.
void __fastcall ObjectDefs_GetFullName(u8 *defs, void *, u32 id, u16 *out, u32 size);
// 0x413020 / 0x4130b0: ObjectText_Name (trim 0) / ObjectText_Suffix, else the definition's +0x4f8 / +0x4fc text (wcsncpy; no definition
// faults, sic).
void __fastcall ObjectDefs_GetName(u8 *defs, void *, u32 id, u16 *out, u32 size);
void __fastcall ObjectDefs_GetSuffix(u8 *defs, void *, u32 id, u16 *out, u32 size);

// 0x51ac20 (stdcall, pops 4): out's 0x80 dwords zeroed; for each id 1-0x100 marked built in the bitset 0x802080 that has a definition (+0x94 the id, walked from the first each time), the number in its file name
// (+0x28c, from the first digit after the last backslash: a name without either faults, sic) appended as a word. Returns 0. Name invented.
s32 __stdcall ObjectDefs_ListBuiltNumbers(u16 *out);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const u8[0x28], gObjectdefs_6fd880, 0x006fd880, 0x28);
GAME_DATA(u8[0x5a00], gTrackType_GetSchema_739ea8, 0x00739ea8, 0x5a00);
GAME_DATA(u8[0xac], gObjectDef_LoadText_7487f8, 0x007487f8, 0xac);
GAME_DATA(u8[0x4ce8], gObjectdefs_76e300, 0x0076e300, 0x4ce8);   // (351 records of 0x38)
GAME_DATA(u8, gObjectdefs_fb3b78, 0x00fb3b78, 1);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[20], gText_ObjectControls_Serialize_75a3a0, 0x0075a3a0, 0x14);
GAME_DATA(char[20], gText_ObjectControls_Serialize_75a3b4, 0x0075a3b4, 0x14);
GAME_DATA(char[20], gText_ObjectControls_Serialize_75a3c8, 0x0075a3c8, 0x14);
GAME_DATA(char[12], gText_ObjectDefs_ListBuiltNumbers_762b18, 0x00762b18, 0xc);
