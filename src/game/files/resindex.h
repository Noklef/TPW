// The resource index (the class names are invented): the font manager's (0x78b530, +0xc) resource folder and the index read from its
// "ResIdx.dat" (0x782d14).
// ResFolder: +0 the index (a ResIndex or null), +4 the folder handle (a Handle, vtable 0x6fd9e8), +0xc 1 once opened.
// ResIndex (vtable 0x70a128; the counted base 0x70a078, its count at +4): +8 the records (12 bytes each: +0 a key, the keys strictly
// increasing; +4 a 16-bit kind, +6 a 16-bit offset, +8 the object loaded for it), +0xc the last record, +0x10 the folder handle (a
// Handle, vtable 0x6fd9e8). Its slots 0 and 1 are the counted base's Release and AddRef (refcounted.h).
// Reimplementation of 0x6ad130-0x6ad458, 0x6af810-0x6af953 and 0x6afa30-0x6afc56.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ad130 (thiscall): the folder handle (+4) constructed (Handle_Ctor) with the folder handle vtable 0x6fd9e8, +0xc and the index
// (+0) cleared. Returns the folder.
u8 *__fastcall ResFolder_Ctor(u8 *folder);

// 0x6ad160 (thiscall): closed (ResFolder_Close, its result a local), then the handle destroyed (Handle_Dtor).
void __fastcall ResFolder_Dtor(u8 *folder);

// 0x6ad1b0 (thiscall (&result, path)): -1 when already open (+0xc); else the handle (+4) opened on the path as a folder (Handle_Open28;
// its result in the path's own argument slot): -1 when that fails, else +0xc set to 1 and 0. Returns &result.
s32 *__fastcall ResFolder_Open(u8 *folder, void *, s32 *result, u32 path);

// 0x6ad200 (thiscall (&result)): the handle (+4) reset through its slot 0, the index (+0) released through its slot 0 and cleared when
// there is one; 0. Returns &result. (+0xc is left set.)
s32 *__fastcall ResFolder_Close(u8 *folder, void *, s32 *result);

// 0x6afbc0 (thiscall (&result, path, records, count)): the handle (+0x10) opened on the path as a folder (Handle_Open28, its result in
// the path's argument slot): -1 when that fails. Else +8 the records, +0xc the last one (records + 12 * count - 12, 32-bit); the first
// record's object (+8) cleared and each record after it up to the last (+0xc read again each time, unsigned) checked: its key above the
// one before (unsigned), its object cleared; a key not above it frees the records (Mem_Free of +8, sic: the caller's records lie inside
// the index's own block) and clears +8: -1. Else 0. Returns &result.
s32 *__fastcall ResIndex_Init(u8 *index, void *, s32 *result, u32 path, u8 *records, u32 count);

// 0x6af810 (thiscall): the counted base constructed (Class70a078_Ctor), the folder handle (+0x10) constructed (Handle_Ctor) with the
// vtable 0x6fd9e8, the vtable 0x70a128, +8 and +0xc cleared. Returns the index.
u8 *__fastcall ResIndex_Ctor(u8 *index);

// 0x6af8a0 (thiscall): the vtable 0x70a128, the records released (ResIndex_ReleaseAll), the handle destroyed (Handle_Dtor), the base's
// vtable 0x70a078.
void __fastcall ResIndex_Dtor(u8 *index);

// 0x6af880 (thiscall (flags); slot 2): ResIndex_Dtor, freed (Mem_Free) for flags bit 0. Returns the index.
u8 *__fastcall ResIndex_DeletingDtor(u8 *index, void *, u32 flags);

// 0x6af900 (thiscall (record); the index unused): with a record, its object (+8) told slot 3 when there is one, and cleared.
void __fastcall ResIndex_ReleaseEntry(u8 *index, void *, u8 *record);

// 0x6af920 (thiscall): with records (+8), each up to the last (+0xc, read again each time, unsigned) released (ResIndex_ReleaseEntry)
// and +8 cleared; then the handle (+0x10) reset through its slot 0.
void __fastcall ResIndex_ReleaseAll(u8 *index);

// 0x6afa30 (thiscall (&result, record, &object); the index unused): with a record, its object (+8) given a reference (slot 1) when
// there is one, then *object = +8 (read again): 0 when not null; -1 without one or without a record (*object then left as it was).
// Returns &result.
s32 *__fastcall ResIndex_GetEntry(u8 *index, void *, s32 *result, u8 *record, u8 **object);

// 0x6afb70 (thiscall (&result, stream, record)): the record's file opened on the stream through the handle (+0x10) slot 3 (&slot, stream,
// the records (+8) + the record's 16-bit name offset (+6), 0x80000001, 0; its result in the stream's own argument slot): -1 when below
// 0, else 0. Returns &result.
s32 *__fastcall ResIndex_OpenEntry(u8 *index, void *, s32 *result, u32 stream, u8 *record);

// 0x6afa70 (thiscall (&result, record, &object); an SEH frame): *object cleared; -1 for a record kind (+4, 16-bit) of 6 or more. Else a
// local stream (StreamBase_Ctor) opened on the record (ResIndex_OpenEntry, its result in the object argument's slot); then the kind's
// loader ([0x782d20 + 4 * kind], read again) told slot 0 (&slot, &stream, &object, the record's key, the index): with no loader or
// either below 0 -1; else the record's object (+8) = *object, told slot 4 (sic: a null one faults), and 0. The stream destroyed
// (StreamBase_Dtor) last. Returns &result.
s32 *__fastcall ResIndex_Load(u8 *index, void *, s32 *result, u8 *record, u8 **object);

// 0x6ad2a0 (thiscall (&result, path); an SEH frame): the old index (+0) released through its slot 0 and cleared; a local folder handle
// (Handle_Ctor, vtable 0x6fd9e8) opened on the path (Handle_Open28): -1 when that fails. Else "ResIdx.dat" (0x782d14) opened through it
// on a local stream (Handle_Vfn3 (0x80000001, 0)) and its 8-byte header read through the stream's source (slot 1; none: -1): "BFRI" and a
// record count. The size (StreamBase_Size) less the position (StreamBase_Tell) is the records' bytes. A count of 0 closes the folder
// (ResFolder_Close, its result in the path's slot): -1. Else a block of the bytes + 0x18 (Mem_Alloc; none: -1) with the records read
// to +0x18 (all of them or the block freed: -1), a ResIndex constructed in it (ResIndex_Ctor) and initialised (ResIndex_Init on the path,
// the records and the count; below 0: the block freed (sic: not destroyed), -1); then it is the folder's index (+0): 0. The stream and
// the handle destroyed. Any failure (but the handle's open) keeps -1. Returns &result.
s32 *__fastcall ResFolder_LoadIndex(u8 *folder, void *, s32 *result, u32 path);

// 0x6ad230 (thiscall (&result, name)): the name's full path (0x200 bytes on the stack) through the handle (+4) slot 5 (&r, buffer,
// name, 0x200): -1 below 0, else ResFolder_LoadIndex on it. Returns &result.
s32 *__fastcall ResFolder_Load(u8 *folder, void *, s32 *result, u32 name);
}
