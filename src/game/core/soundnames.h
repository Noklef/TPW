// The sound names' table (the singleton at 0xfb1f28, made by 0x6beb20 behind the guard 0x6bead0; the class name is invented): +0x3c and
// +0x4c a deque's begin and end iterators ({block start, block end, current, map node}; blocks of 0x400 pointers), each element an object
// whose String at +0xc (length +0x10, text +0x14) is a name; +0x64 a count. Reimplementation of 0x6c0dc0; the name is unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c0dc0 (thiscall (out, stream), pops 8; called from 0x6b6cb2): the table saved to a stream (vtable: +0xc write (buffer, count), +0x10
// seek (offset, origin), +0x18 tell): the position, the count (+0x64) written twice, that many zero dwords, then for each element a byte
// (its name's length + 1, 0 for a null element) and, when that byte is not 0, that many bytes of the name's text; then the size from the
// first position less 8 written there (seek to it, write, seek back to the end). *out = 0. Returns out.
u32 *__fastcall SoundNames_Save(u8 *self, void *, u32 *out, u8 *stream);

// The loaded names' map (the singleton's +4; names invented): +0 a count, +4 a block (Mem_Alloc) holding that many dwords (each saved
// name's record index in the singleton's deque A, SoundCategory_FindRecordIndex; 0 for a null name), then the names read with them.
// 0x6c10f0 (thiscall): the block (when there is one) freed (Mem_Free) and +4 = 0 (the count kept).
void __fastcall SoundNameMap_Free(u8 *map);
// 0x6c0d90 (thiscall): SoundNameMap_Free on the singleton's +4.
void __fastcall SoundNames_FreeMap(u8 *self);
// 0x6c1050 (thiscall (out, stream), pops 8): SoundNameMap_Free, then from the stream (vtable +8: read (buffer, count)) a size, the count
// (+0) and size bytes into a new block (+4; Mem_Alloc, not checked). The names follow the count's dwords there: each a length byte and,
// when that is not 0, that many bytes, the dword the record index for the text after the byte (on the singleton, SoundCategory_Get), else
// 0. *out = 0. Returns out.
u32 *__fastcall SoundNameMap_Load(u8 *map, void *, u32 *out, u8 *stream);
// 0x6c0f50 (thiscall (out, stream), pops 8): SoundNameMap_Load on +4 (its out the stream argument's own slot, sic); *out = 0. Returns out.
u32 *__fastcall SoundNames_Load(u8 *self, void *, u32 *out, u8 *stream);

}  // extern "C"
