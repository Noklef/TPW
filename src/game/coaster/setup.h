// A coaster type set up from its description (0x430000 builds one): the setup 0x42f390 and
// its pylon controls and cars and trains steps (the cross sections are in xsection.h). Each
// step fails with 0, after which the setup reports it to the empty Debug_Printf and fails.
// "Non-zero" floats are by the x87 compare against 0: NaN counts as zero.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// What CoasterPiece_Load loads with (default Ride_LoadResource; the bench's recorder; nullptr restores it).
using CoasterPieceLoad = u32(__cdecl *)(const char *folder, const char *name, const u8 *cells, const u8 *grid, u32 flags, u32 id, u32 layers, const u8 *def, u32 signArg, u32 width, u32 height);
void CoasterPiece_SetLoad(CoasterPieceLoad load);
// CoasterType_Load's calls by address (the bench's recorders).
void CoasterType_SetLoadCalls(const AddrCall *table, u32 count);

extern "C" {
// 0x42fad0 (cdecl; slot, folder, source; name invented, unconfirmed): a coaster type read for a definition: CoasterType_Create (slot, folder, "<folder>\GTexture", source); bit 0x40000 set in the
// holder's dword +4 (the table 0x7ae6a8), the largest +0x3c (signed, 0x790fd0) and part +0x1c's word +4 (unsigned, 0x791cfc) kept; the holder at +0x14 given its sizes, zero and 800.0 vectors; the animator
// (+0x18, +0x1c, "TrackMaster", 1), on failure Dword_ClearBit0 of +0x1c; Matrix_Identity of its matrix; the three pieces (+0x300, then +0x2fc and +0x304 unless the same as the first) loaded with their
// three lengths and the total for +0x2f4 sections; the optional parts +0xa8 and +0x1c8 (a name at +0x10): Ride_LoadResource, the model read, flags 1 / 2; +0x318 the total times +0x2e8.
void __cdecl CoasterType_Load(u32 slot, const char *folder, const u8 *source);

// 0x430130 (cdecl; piece, folder; name unconfirmed; one of the three of a type, +0x300, +0x2fc, +0x304): Ride_LoadResource (folder, the name +8, flags 0xc0, id +4, one layer) gives +0xc, its slot in the table 0x7ae6a8;
// 0 or an empty slot end here. +0x28 = +0x14 - (the model's bound +0x88 - the float +0x48 of its +0x78), +0x24 = +0x10 + (bound +0x94 - that); +0x30 = its parts with flag 0x80 and +0x2c a GlobalMem list of them (0 for none).
// With the piece's bit 0: the model's bounds +0x80 and those (+0x78) of its face (+0x70, 0xa0 each, the first +0x44) with the greatest u16 +0x58 widened to the longer corner's length + 4 on all sides, that face's flag 0x200000 set, its index to +0x20.
void __cdecl CoasterPiece_Load(u8 *piece, const char *folder);

// 0x430ad0 (cdecl; name unconfirmed): every coaster type of the table 0x790b08 (the count 0x790fe8) freed, then the count 0. For each:
// the GlobalMem blocks +0x2c of its three pieces +0x300, +0x2fc and +0x304 (the latter two only when not the first), +0x308, the
// names (+4) of its entries (c +0x1c: u16 +4 of them less the static count 0x74ce10, 0x10 bytes each from +0x10), then c's bit 0
// cleared (Dword_ClearBit0), b (+0x18) disposed of as an animator (Animator_Dispose), c's entries, c, b, a (+0x14), the block +0xc
// of the +0x8c-th 0x18-byte record of +0x88, +0x90, +0x94, +0x98, +0x88 and +0x78; each pointer cleared (but the record's), and the
// table's slot.
void __cdecl CoasterTypes_FreeAll();

// 0x42f390: +0x3b08 (when non-zero) to +0x40 first; 0 when the description has no track
// entries (+0xfc). Else four GlobalMem blocks: a (0x30; +0 = 3), b (0x40; +0 = 3, u16 +4 = 1,
// +8 = a), c (0x20; +0 = 0, u16 +4 = the entry count, +8 = id, +0x10 = the entries) and the
// entries (0x10 each): one per track entry (0xc bytes at +0xc: name, flag, float) with +0 6
// (flag) or 2, |0x20 and +0xc = the float when non-zero (and coaster flag 4), +4 a GlobalMem
// copy of the name; then the static ones at 0x74cde0 (count 0x74ce10). The coaster gets a
// (+0x14), b (+0x18), c (+0x1c), the count (+0x20), then CrossSection_Setup,
// CoasterPylons_Setup and CoasterTrains_Setup (0 if one fails), the +0x3ac8 pairs at +0x3a98
// in a GlobalMem block (+0x9c, count +0xa0), non-zero fields (+0x3ae8, +0x3aec -> +0x2e8,
// +0x2ec; floats +0x3ae4 -> +0xa4, +0x3acc/+0x3ad0 -> +8/+0xc, +0x3ad8..+0x3ae0 -> +0x24..+0x2c,
// +0x3b0c..+0x3b18 -> +0x44..+0x50, +0x3b28..+0x3b34 -> +0x60..+0x6c, +0x3b38..+0x3b70 ->
// +0x320..+0x358, +0x3b1c..+0x3b24 -> +0x54..+0x5c; +0x3b78 -> +0x30 and +0x34, +0x3b7c ->
// +0x3c), +0x3b74 (+0x31c) and its flags: +0x3af0 sets 0x8000 (clearing 0x4000) in +0x1c8 and
// 0x100 in +0; then 0xa0, 0x10 unless +0x3af8, and 8 (clearing 0x10) when +0x3af4. Then 1.
u32 __cdecl CoasterType_Setup(u8 *coaster, u32 id, const u8 *source);

// 0x42fd90 (cdecl (holder, unused, id, source); name unconfirmed; CoasterType_Load 0x42fad0 calls it): a new coaster type, a
// zeroed GlobalMem block of 0x35c bytes added to the table 0x790b08 (count 0x790fe8), with the holder (+4), its defaults (floats
// and counts across +8..+0x358, the two sets of five {0, 1, 1, 0} at +0x178 and +0x298, +0x9c / +0xa0 from 0x74cac8, the roots
// of the doubles 0x6fde80 (+0x14c, +0x26c), 0x6fde88 (+0x150) and 0x6fde90 (+0x270), flags +0x1c8 0x4004), then CoasterType_Setup
// from the source (its result unused) and c's (+0x1c) texture path +0xc the string 0x74cb98. A track type balance file (vtable
// 0x6fd8b0) is laid out on the stack first and released at the end, used for nothing (sic). Returns the type.
u8 *__cdecl CoasterType_Create(u32 holder, u32 unused, u32 id, const u8 *source);

// 0x42eb70: for each of source + 0x3a20 descriptions (0xbc bytes at source + 0x38a8; two fit
// before the count; the first fills the control at coaster + 0xa8, every later one the one at
// coaster + 0x1c8): 0 unless the description has a name (+0), +4 and a float +0xc that are
// non-zero. The control gets a GlobalMem copy of the name (+0x10), +4 (+0xc), flag 0x10 when
// +8 is non-zero, +0xc (+0x58) and +0x60..+0x68 (+0x5c..+0x64); the rest only when non-zero:
// floats +0x10 (+0xb0), +0x20..+0x28 (+0x68..+0x70), +0x6c..+0xb8 (+0xd0..+0x11c), +0x2c..+0x50
// in degrees (+0x74..+0x98, as radians: * 0x6fde78), +0x5c (+0xac, * 10 * 0x6fde78); u32s
// +0x14, +0x18, +0x1c (+0xb4, +0xb8, +0xcc); ints +0x54 and +0x58 squared (+0x9c, +0xa0) and
// as floats (+0xa4, +0xa8). 1 when all have them (or there are none).
u32 __cdecl CoasterPylons_Setup(u8 *coaster, const u8 *source);

// 0x42f1a0: 0 when source + 0x3a78 has no cars. Otherwise a GlobalMem array of that many cars
// (0x34 bytes) from the descriptions (0x1c bytes at source + 0x3a24; three fit before the
// count): each car cleared to flags 0 (+0), +4, name (+8), +0x10..+0x18 zero and +0x1c 0.1f
// (+0xc left alone), then 0 (the array kept by nobody, sic) unless the description has +4 and
// a name (+8); flag 1 when +0 is non-zero, +4, a GlobalMem copy of the name, +0xc..+0x14
// (+0x10..+0x18) and +0x18 when non-zero (+0x1c). The coaster gets the array (+0x308), the
// cars numbered at source + 0x3a80, + 0x3a84 and + 0x3a88 (+0x2fc, +0x300, +0x304; flags
// +0x2f0: 1 when the first differs from the second, 2 when the third does), source + 0x3a7c
// when non-zero, else 3 (+0x2f4), and source + 0x3a8c..+0x3a94 (+0x30c..+0x314). Then 1.
u32 __cdecl CoasterTrains_Setup(u8 *coaster, const u8 *source);

// 0x43ba20 (coaster, count): nothing when count is 0. Otherwise, with a bit of 0xc in the flags at +8, the cars from count up to the current count (+0xf4) are unloaded on both
// sides (CoasterTrain_Unload, the trains of 0x80 bytes from the pointer at +0x128); then +0xf4 is count.
void __cdecl Coaster_SetTrainCount(u8 *coaster, u32 count);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const double, gCoasterType_Create_6fde80, 0x006fde80, 0x8);
GAME_DATA(const double, gCoasterType_Create_6fde88, 0x006fde88, 0x8);
GAME_DATA(const double, gCoasterType_Create_6fde90, 0x006fde90, 0x8);
GAME_DATA(s32, gSetup_790fd0, 0x00790fd0, 0x4);
GAME_DATA(u8 *[50], gCoasterTypes_790b08, 0x00790b08, 0xc8);   // the coaster types (count 0x790fe8)
GAME_DATA(u32, gSetup_790fe8, 0x00790fe8, 0x4);
GAME_DATA(u32, gCoasterLinkWordsMax_791cfc, 0x00791cfc, 0x4);   // the links' largest word count (CoasterType_Load)

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x110], gSetup_74ca70, 0x0074ca70, 0x110);
GAME_DATA(char[0xc], gText_CoasterType_Load_74cb80, 0x0074cb80, 0xc);
GAME_DATA(char[0xc], gText_CoasterType_Load_74cb8c, 0x0074cb8c, 0xc);
GAME_DATA(u32[3][4], gCoasterType_Setup_74cde0, 0x0074cde0, 0x30);   // the static entries (7, a texture name's address, 0, 0)
GAME_DATA(u32, gCoasterType_Setup_74ce10, 0x0074ce10, 0x4);          // their count
