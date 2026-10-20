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

// 0x5b0cd0 (thiscall; name unconfirmed): a theme's record (the global save's theme map holds them; 0x5af290 and 0x5b0bc0 delete
// them): its theme balance file (+0x1c, a heap object, vtable 0x702800 its schema getter Table73f8a8_At), when there is one, gets
// its vtable back, is released (BalanceFile_Release) and freed (Mem_Free); then its name (a String at +0x20) is destroyed.
void __fastcall ThemeRecord_Dtor(u8 *self);

// 0x415e80 (thiscall; name unconfirmed): the balance file at +0xc gets vtable 0x6fd958 back and is released (BalanceFile_Release).
void __fastcall AdvisorScoring_Dtor(u8 *self);

// 0x59bf10 (thiscall; name unconfirmed): the advisor scoring balance file itself (the one at +0xc, an unwind action of
// AdvisorScoring_Construct) gets its vtable 0x6fd958 back and is released.
void __fastcall AdvisorScoringFile_Dtor(u8 *self);

// 0x415e70 (cdecl, at exit; name unconfirmed): AdvisorScoring_Dtor of gAdvisorScoring (0xfb3540).
void __cdecl AdvisorScoring_StaticDtor();

// 0x59bea0 (thiscall; the function-static gAdvisorScoring's constructor, under the guard 0xfb2766 & 1 with AdvisorScoring_StaticDtor
// given to atexit by its callers): the balance file at +0xc gets vtable 0x6fd958 and is laid out (BalanceFile_Init), +0 = 0, and
// "data/advisor/advisor.sam" (0x773014) loaded into it (BalanceFile_Load, keep 0; its failure only goes to the empty Debug_Printf).
// Returns the object.
u8 *__fastcall AdvisorScoring_Construct(u8 *self);

// 0x402720 (cdecl; &text, &map, &error, symbols, count; name unconfirmed): a map description read from the text. *error = 0; the text's
// leading white space (the CRT's iswspace of each char sign-extended to 16 bits) and the rest of that line are passed over; then a line of
// "---" (at least), then rows up to a line starting "---": each character but spaces is looked up among the count symbols (0xc bytes: +0
// the char, +4 a dword, +8 a byte; a count below 0 takes the first, sic) and becomes the next cell (+4's dword and +8's byte) of the row;
// the width is the longest row. Errors (to *error, 0 returned): no line end "Missing map description" (0x739b80), no "---" "...initial
// marker" (0x739b08), the text's end before the closing marker "Missing map delimter" (0x739b68), a char in the 21st row "Map too high"
// (0x739b58; blank rows past the 20th go unchecked and overrun the grid, sic), an
// unknown char "Illegal map character" (0x739b40), a 21st cell "Map too wide" (0x739b30). Done, the rows are turned upside down and the
// 0xc88-byte map (+0 width, +4 height, +8 20 x 20 cells of 8 bytes, zeros elsewhere) copied into a Mem_GlobalAlloc block (none:
// GameError_Set(0x785de8, 8, 0, 0), 0) that goes to *map; *text moves past the closing marker's line; 1.
u32 __cdecl BalanceFile_ParseMap(const char **text, u8 **map, const char **error, const u8 *symbols, s32 count);

// 0x4017a0 (thiscall (name, text, keep), pops 12): a balance file's text into its values; 1, or 0 at the first error (its message,
// with the file's name, went to Debug_Printf: left out). keep 0 sets the defaults first (BalanceFile_SetDefaults(0)). The text's CRs
// are removed in place. Lines, each from its first char past white space (the CRT's isspace, across line ends too, sic), are
// skipped when that is the end, '#' or '\n'; else "Group.Field value" or "Group[row].Field.Field value value": a group name of
// alphanumerics (up to 64), '.' or '[' digits (row, atoi) "]."; 1-16 fields of up to 64 alphanumerics, each starting with a
// letter, joined by '.' and ended by white space (a 17th field is an error); then each field's key is found (BalanceFile_FindKey;
// grouped with a row) and its value read: a map (type 11, the line's only field) parsed from the line's start
// (BalanceFile_ParseMap with the symbols at 0x7396c8 (19) or 0x7397b0 (16) as the key record's +0x38 is 1 or 2; another
// map_type an error whose message reads an uninitialised pointer, sic, and a failed map with no error text faults in its
// strcat, sic), a quoted string (type 10) on the line copied into a Mem_GlobalAlloc block, a float (types 7-9: [-]digits and at
// most one point, under 100 chars, atof; type 8 at least 0, type 9 from +0x2c up to +0x30) or else an integer ([-]digits, atoi;
// type 5 at least 0, type 6 from +0x24 up to +0x28), each ended by white space or the text's end. A value goes to row of the key's
// entry when grouped (its row count at +8 raised to row + 1, even when the value is then out of range) or to its single value; a
// map or string frees the block it replaces (Mem_GlobalFree). The next line starts past the next '\n' (after a map, the one
// after its closing marker's line, sic).
u32 __fastcall BalanceFile_Parse(u8 *self, void *, const char *name, char *text, u32 keep);

// 0x401650 (thiscall (path, keep), pops 8; name unconfirmed): the balance file at path parsed (BalanceFile_Parse, keep passed on)
// from a Mem_GlobalAlloc block of its size (File_GetSize) + 1, loaded (File_Load) and freed after. No file: 0, with
// GameError_Set(0x785de8, 3, 0, path) unless keep; an empty one: 1; no block or a failed load: that error (keep or not) and 0
// (a loaded block not freed, sic).
u32 __fastcall BalanceFile_Load(u8 *self, void *, const char *path, u32 keep);

// 0x401700 (thiscall (name, stream, keep), pops 12; name unconfirmed): the same from a stream: its size (vtable +0x14) above 0,
// a block of it + 1 read (vtable +8 (block, size)) whole (unsigned), else GameError_Set(0x785de8, 3, 0, name) (8 when the
// allocation fails) and 0, the read block not freed (sic).
struct Stream;
u32 __fastcall BalanceFile_LoadStream(u8 *self, void *, const char *name, Stream *stream, u32 keep);

// 0x5b0d40 (stdcall; i): the address of 60-byte record i of the table at 0x73f8a8.
u32 __stdcall Table73f8a8_At(u32 i);
}
