// The sound system's reverb presets (SndReverb.map in its folder): reimplementation of their load, 0x6c5d20-0x6c5ff8.
//
// The object (the sound system, whose +0x30 String is its folder): +0x68 the presets (a heap block of 0x34-byte records, 13 dwords each; a
// first dword of -1 marks a slot the file leaves empty), +0x6c how many.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c5d20 (thiscall): with no presets yet (+0x68 0), SndReverb.map is opened in the folder (+0x30; FileStream_OpenPath, mode 0x80000001) and read
// through its source's vtable +4: the version into +0x6c (2000, else nothing more), then the count over it, a zeroed block of count records
// (Mem_Alloc) into +0x68 and for each record a byte: 0 gives the record's first dword -1, else 13 dwords are read into it (through a local, so a
// short read leaves the last record's values, sic). The stream is closed after a good version.
void __fastcall SndReverb_Load(u8 *system);

}  // extern "C"
