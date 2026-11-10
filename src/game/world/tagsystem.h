// The tag system (thing model 12) and its CTagTable (at +0x80c of the tag system; the name is the one its "not found"
// message gives): reimplementation of the table's lookup 0x50af40. The table itself is static data, 0x17e records of
// 0x20 bytes at 0x75eca8-0x761c68, each starting with its id.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x50af40 (thiscall (id), pops 4; the table object is not read): 0 for the ids 0x17e and 0x17f; the record at
// 0x75eca8 + id * 0x20 (any id, the address wrapping) when its id is id; else the first record of the table with that id,
// or 0 (after a "CTagTable - not found" debug print, not reproduced).
u32 *__fastcall TagTable_Find(void *self, void *, u32 id);

// 0x509b40 (thiscall): the thing constructor (Thing_Construct, cell 1, kind 0xc), then the tag system added to the message centre's slots
// 7, 0xc, 0x14, 0x18, 0x19 and 0x13 in that order. Returns it. (The cell the original pushes has this's high address bits above the 1.)
u8 *__fastcall TagSystem_Ctor(u8 *tags);

// 0x509e10 (thiscall (record, a, b, c), pops 16; returns AL; name invented, unconfirmed): the tag's text into this +0xc (0x400 characters) by the
// record's kind +0x1c, 0-0x24 (any other: nothing, 1): the text +0xc of the table [0x78ba90] (0x6acd60) with %n arguments from a / b (a word) / the
// globals 0x785790-0x7857a8 / the record c (+0x14 to +0x24, signed numbers; the object names of its words +0x28, +0x2a (0x412fb0, 0x40 characters);
// the text 1, 3, 5, 7, 9 of [0x78bac0] by its kind +0x2c (above 4: the buffer as it was, sic, the original's uninitialised stack)), or a fixed L"" text
// through swprintf. No record: the text 0x78a374, 0. The kind 1 without a's slot (0x7cfb90 + a * 20) or the kind 2 without b's word: 0 (nothing written).
u8 __fastcall TagSystem_FormatTag(u8 *self, void *, const u8 *record, u32 a, u32 b, const u8 *c);

// 0x509bf0 (thiscall (message), pops 4; name invented, unconfirmed): the tag's message: the message's type (slot 0) 0x13 its +8 7, 8 or 9 gives the tag id
// 0x17b-0x17d (OutputDebugStringA of a line), 0x18 and 0x19 its +8; the record (0x50af40; none: nothing) formatted (a word of +0xc and b +0x14 for 0x18,
// c the message for 0x19), then posted (0x481580: the id, the record's +4, this +0xc, +0x18 or for 0x18 the message's +0x10 when not negative, word +8,
// +0x10, +0x14; for 0x18 the message's word +0xc and +0x14 too). Other types: slot 0 asked again (for a debug print, left out).
void __fastcall TagSystem_HandleMessage(u8 *self, void *, u8 *msg);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[32], gText_TagSystem_HandleMessage_75eb20, 0x0075eb20, 0x20);
GAME_DATA(char[40], gText_TagSystem_HandleMessage_75eb40, 0x0075eb40, 0x28);
GAME_DATA(char[36], gText_TagSystem_HandleMessage_75eb68, 0x0075eb68, 0x24);
GAME_DATA(u16[28], gText_TagSystem_HandleMessage_75ebe0, 0x0075ebe0, 0x38);
GAME_DATA(u16[33], gText_TagSystem_HandleMessage_75ec64, 0x0075ec64, 0x42);
