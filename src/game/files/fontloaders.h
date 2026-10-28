// The text library's two font kinds past the character tables (charsets.h; the class names are invented), each loaded by its kind's
// loader (the table of loaders at 0x782d20; thiscall (&result, stream, &object, key, index), pop 20, `this` unused) and each on the counted
// base (vtable 0x70a078, its count at +4; refcounted.h):
// BitmapFont (vtable 0x70a140, a "BFMB" file; 12 bytes): +8 the lead byte, +0xa a 16-bit word of the header.
// Font (vtable 0x70a158 over the base 0x70a208, an "F4FB" file): +8 and +0xc the file's data, +0x10 its data + 8, +0x14 the index's
// CodeTable (key 1, kind 0) held, or null.
// Reimplementation of 0x6b0340-0x6b0429, 0x6b0480-0x6b050a, 0x6b0680-0x6b0708 and 0x6b4a10-0x6b4a21.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b0340 (the "BFMB" loader; an SEH frame): the 8-byte header read through the stream's slot 2 (into 12 bytes on the stack): unless 8
// bytes and "BFMB", the stream told its slot 1 and -1. Else a 12-byte block (Mem_Alloc): the counted base constructed (Class70a078_Ctor),
// the vtable 0x70a140, +8 0xff less header byte 4, +0xa header bytes 6-7; *object the block, 0 (no block: *object null and 0, sic). The
// rest of the stream is not read. Returns &result.
s32 *__fastcall BitmapFont_Load(u8 *loader, void *, s32 *result, u8 *stream, u8 **object, u32 key, u8 *index);

// 0x6b4a10 (thiscall): the counted base constructed (Class70a078_Ctor), the vtable 0x70a208. Returns the object.
u8 *__fastcall FontBase_Ctor(u8 *font);

// 0x6b0480 (thiscall (data, index), pops 8; an SEH frame): FontBase_Ctor, +8 and +0xc the data, +0x10 the data + 8, the vtable 0x70a158;
// the index's record for key 1 (ResIndex_Find): of kind 0 (Record_Word4OrSix), its object fetched into +0x14 (ResIndex_GetEntry, its
// result in the data's slot), else +0x14 null. Returns the font.
u8 *__fastcall Font_Ctor(u8 *font, void *, u8 *data, u8 *index);

// 0x6b0680 (kind 4's loader, the "F4FB" file): the stream's size (slot 5, no position taken off) the data's bytes: a block of them + 0x18
// (Mem_Alloc; none: -1) with the data read to +0x18 through slot 2; unless all of them were read and the data starts with "F4FB" the block
// freed and -1. Else a Font constructed in the block on that data and the index (Font_Ctor), *object the block, 0. Returns &result.
s32 *__fastcall Font_Load(u8 *loader, void *, s32 *result, u8 *stream, u8 **object, u32 key, u8 *index);
}
