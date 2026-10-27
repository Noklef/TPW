// The resource index (the class names are invented): the font manager's (0x78b530, +0xc) resource folder and the index read from its
// "ResIdx.dat" (0x782d14).
// ResFolder: +0 the index (a ResIndex or null), +4 the folder handle (a Handle, vtable 0x6fd9e8), +0xc 1 once opened.
// ResIndex (vtable 0x70a128; the counted base 0x70a078, its count at +4): +8 the records (12 bytes each: +0 a key, the keys strictly
// increasing; +4 a 16-bit kind, +6 a 16-bit offset, +8 the object loaded for it), +0xc the last record, +0x10 the folder handle (a
// Handle, vtable 0x6fd9e8). Its slots 0 and 1 are the counted base's Release and AddRef (refcounted.h).
// Reimplementation of 0x6ad130, 0x6ad160, 0x6ad1b0, 0x6ad200 and 0x6afbc0.
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
}
