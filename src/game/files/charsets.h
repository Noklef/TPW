// The text library's character tables (the class names are invented), each loaded from a file the resource index (resindex.h) names by
// its kind's loader (the table of loaders at 0x782d20) and each on the counted base (vtable 0x70a078, its count at +4; refcounted.h).
// RangeTable (vtable 0x70a0e0, a "BFUM" file): +8 the range segments (Ranges_Lookup's, methods.h), the file's data after its magic.
// CodeTable (vtable 0x70a0f8, a "BFMU" or "BFMH" file): +8 the lead byte, +0xa the code count (16-bit), +0xc the words (Wide_FromNarrow's,
// methods.h), the file's data after its 8-byte header.
// The loaders are thiscall (&result, stream, &object, key, index), pop 20, their `this` (a one-slot loader object) unused; the stream's
// slot 2 reads (buffer, bytes) and answers the bytes read, slot 5 answers its size and slot 6 its position.
// Charset (vtable 0x70a060, a "BFST" file): +8 the count of entries, +0xc their table (each a dword offset from the table, made a pointer
// by the constructor; Records_DataIfTerminated, methods.h), +0x10 the index's CodeTable (key 1), +0x14 its RangeTable (key 2), each held
// (a reference) or null.
// Reimplementation of 0x6aca10-0x6acb64, 0x6acec0-0x6acfbf, 0x6ad860-0x6ad8eb, 0x6ad950-0x6ad9f2 and 0x6ada90-0x6adc7a.
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

// 0x6aca10 (thiscall (count, table, index), pops 12; an SEH frame): the counted base constructed, the vtable 0x70a060, +8 the count, +0xc
// the table, each of its count offsets made a pointer (+0xc, read again for each, added). Then the index's record for key 2
// (ResIndex_Find): of kind 1 (Record_Word4OrSix), its object fetched into +0x14 (ResIndex_GetEntry, its result in the count's slot: a
// reference, or null), else +0x14 null; the record for key 1 likewise of kind 0 into +0x10. Returns the charset.
u8 *__fastcall Charset_Ctor(u8 *charset, void *, u32 count, u32 *table, u8 *index);

// 0x6acb00 (thiscall; an SEH frame): the vtable 0x70a060; +0x14 then +0x10 released through their slot 0 and cleared when held; the
// base's vtable 0x70a078.
void __fastcall Charset_Dtor(u8 *charset);

// 0x6acae0 (thiscall (flags), pops 4; slot 2): Charset_Dtor, freed (Mem_Free) for flags bit 0. Returns the charset.
u8 *__fastcall Charset_DeletingDtor(u8 *charset, void *, u32 flags);

// 0x6acec0 (kind 3's loader): the 12-byte header read through the stream's slot 2: -1 unless 12 bytes, "BFST" and the key in its second
// dword. The size (slot 5) less the position (slot 6) the data's bytes: a block of them + 0x18 (Mem_Alloc; none: -1) with the data read
// to +0x18 (not all of them: the block freed, -1), a Charset constructed in it (the header's third dword the count, that data the table,
// the index), *object the block: 0. Returns &result.
s32 *__fastcall Charset_Load(u8 *loader, void *, s32 *result, u8 *stream, u8 **object, u32 key, u8 *index);

}
