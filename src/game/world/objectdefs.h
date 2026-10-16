// The object definitions (loaded by ObjectDefs_Load 0x413c10 from .sam files): reimplementation
// of the lookups 0x412e90, 0x412ef0 and 0x412f50, the walk 0x412e60 / 0x412e70 and the schema slots 0x412c90
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

// The schemas of the .sam files (vtable slots, this unused): record index of 0x3c bytes.
// 0x412c90: an object definition's (0x744b30). 0x4143f0: a track type's (0x739ea8).
const u8 *__fastcall ObjectDef_GetSchema(const void *, void *, u32 index);
const u8 *__fastcall TrackType_GetSchema(const void *, void *, u32 index);
}
