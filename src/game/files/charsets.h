// The text library's character tables (the class names are invented), each loaded from a file the resource index (resindex.h) names by
// its kind's loader (the table of loaders at 0x782d20) and each on the counted base (vtable 0x70a078, its count at +4; refcounted.h).
// RangeTable (vtable 0x70a0e0, a "BFUM" file): +8 the range segments (Ranges_Lookup's, methods.h), the file's data after its magic.
// CodeTable (vtable 0x70a0f8, a "BFMU" or "BFMH" file): +8 the lead byte, +0xa the code count (16-bit), +0xc the words (Wide_FromNarrow's,
// methods.h), the file's data after its 8-byte header.
// The loaders are thiscall (&result, stream, &object, key, index), pop 20, their `this` (a one-slot loader object) unused; the stream's
// slot 2 reads (buffer, bytes) and answers the bytes read, slot 5 answers its size and slot 6 its position.
// Reimplementation of 0x6ad860-0x6ad8eb, 0x6ad950-0x6ad9f2 and 0x6ada90-0x6adc7a.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ad860 (thiscall (data), pops 4; an SEH frame): the counted base constructed (Class70a078_Ctor), +8 the data, the vtable 0x70a0e0.
// Returns the table.
u8 *__fastcall RangeTable_Ctor(u8 *table, void *, u8 *data);

// 0x6ad8c0 (thiscall; an SEH frame): the base's vtable (0x70a078).
void __fastcall RangeTable_Dtor(u8 *table);

// 0x6ad8a0 (thiscall (flags), pops 4; slot 2): RangeTable_Dtor, freed (Mem_Free) for flags bit 0. Returns the table.
u8 *__fastcall RangeTable_DeletingDtor(u8 *table, void *, u32 flags);

// 0x6ad950 (kind 1's loader): the 4-byte magic read through the stream's slot 2 into the stream argument's own slot (sic): -1 unless 4
// bytes and "BFUM". Else the size (slot 5) less the position (slot 6) the data's bytes: a block of them + 0xc (Mem_Alloc; none: -1) with
// the data read to +0xc (not all of them: the block freed, -1), a RangeTable constructed in it on that data, *object the block: 0.
// Returns &result.
s32 *__fastcall RangeTable_Load(u8 *loader, void *, s32 *result, u8 *stream, u8 **object, u32 key, u8 *index);

// 0x6adbe0 (thiscall (lead, count, data), pops 12; an SEH frame): the counted base constructed, +8 the lead's low byte, +0xa the count's low
// word, the vtable 0x70a0f8, +0xc the data. Returns the table.
u8 *__fastcall CodeTable_Ctor(u8 *table, void *, u32 lead, u32 count, u8 *data);

// 0x6adc50 (thiscall; an SEH frame): the base's vtable (0x70a078).
void __fastcall CodeTable_Dtor(u8 *table);

// 0x6adc30 (thiscall (flags), pops 4; slot 2): CodeTable_Dtor, freed (Mem_Free) for flags bit 0. Returns the table.
u8 *__fastcall CodeTable_DeletingDtor(u8 *table, void *, u32 flags);

// 0x6ada90 (kind 0's loader): the 8-byte header read through the stream's slot 2: -1 unless 8 bytes and "BFMU" or "BFMH". The lead byte
// 0xff less header byte 4; the count (16-bit) 0xff * (((header byte 6 - 1) & 0xff) + 0x101) - 0xfe * lead + header byte 7 (the code
// Wide_FromNarrow gives the pair of bytes 6 and 7, sic: 0x200 for a byte 6 of 0). The size (slot 5) less the position (slot 6) the data's
// bytes: a block of them + 0x10 (Mem_Alloc; none: -1) with the data read to +0x10 (not all of them: the block freed, -1), a CodeTable
// constructed in it (the lead, the count, that data), *object the block; then -1 for "BFMH" (sic: the table made and handed over), else 0.
// Returns &result.
s32 *__fastcall CodeTable_Load(u8 *loader, void *, s32 *result, u8 *stream, u8 **object, u32 key, u8 *index);

}
