// The sound stream (vtable 0x70a778; the names are invented), the base of the stream filter (src/game/core/streamfilter) and the
// sound sources: a stream in a chain, +4 the next stream downstream, +8 the one upstream, +0xc an id. Its slots: 0 the destructor, 1 the
// next set, 2 the close, 3 the unlink, 4 the find by id, 5 a description, 6 the sample rate, 9 a query (out, record), 11 a send; the
// chain's slots pass on to the next stream. Reimplementation of 0x6bab30, 0x6ba9a0, 0x6c61e0-0x6c6300 and 0x6c6d70.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6bab30 (thiscall (flags), pops 4; slot 0): the vtable 0x70a778; Mem_Free when flags bit 0. Returns the stream.
u8 *__fastcall SoundStream_Dtor(u8 *stream, void *, u32 flags);
// 0x6c61e0 (thiscall (next), pops 4; slot 1): with a next, the old next closed (its slot 2) when there was one, then linked both ways
// (+4, the next's +8). Returns the next.
u8 *__fastcall SoundStream_SetNext(u8 *stream, void *, u8 *next);
// 0x6c6210 (cdecl (next, stream)): the stream's slot 1 (next); returns the stream.
u8 *__cdecl SoundStream_Chain(u8 *next, u8 *stream);
// 0x6c6230 (thiscall; slot 2): the next closed (its slot 2) and forgotten, then the stream's slot 0 (1), unless the stream is null.
void __fastcall SoundStream_Close(u8 *stream);
// 0x6c6300 (thiscall; slot 3): with one upstream (+8), the next's +8 = it (with a next) and its +4 = the next (re-read; else 0); then
// the stream's slot 0 (1), unless the stream is null.
void __fastcall SoundStream_Unlink(u8 *stream);
// 0x6c62e0 (thiscall (id), pops 4; slot 4): the stream when its +0xc is the id, else the next's slot 4 (id), else 0.
u8 *__fastcall SoundStream_Find(u8 *stream, void *, u32 id);
// 0x6c62c0 (thiscall (record), pops 4; slot 5) and 0x6c6260 (thiscall (value), pops 4; slot 11): the next's slot 5 / 11, with a next.
void __fastcall SoundStream_Describe(u8 *stream, void *, u8 *record);
void __fastcall SoundStream_Send(u8 *stream, void *, u32 value);
// 0x6ba9a0 (thiscall; slot 6): the next's slot 6, or 0 without one.
u32 __fastcall SoundStream_SampleRate(u8 *stream);
// 0x6c6280 (thiscall (out, record), pops 8; slot 9): *out = the dword at the pointer the next's slot 9 (a local, the record) answers (the
// local is the record's own stack slot), or 0 without a next. Returns the out.
u32 *__fastcall SoundStream_Query(u8 *stream, void *, u32 *out, u8 *record);
// 0x6baab0 (thiscall (flags), pops 4; slot 0 of 0x70a6c8): the base's vtable (Vtbl70a778_Set_6c7090), then, when flags bit 0, the
// pool's delete (Stream70a6c8Pool_Delete, 0x1c). Returns the stream.
u8 *__fastcall Stream70a6c8_Dtor(u8 *stream, void *, u32 flags);
// 0x6c6d70 (thiscall (out, ms), pops 8): without a next stream (+4) *out = -1. Else the byte offset of the time: the sample rate (slot 6)
// times (ms times 0.001f) (x87 at 24 bits, through a float), doubled when slot 17 answers 2 (AL) and again when slot 16 answers 0x10,
// rounded (fistp); its low two bits, when not 0, added to it (sic). Slot 12 (seek) to it when it is not past the length (slot 14,
// unsigned), else to it modulo the length unless that is 0. *out = 0. Returns out.
u32 *__fastcall SoundStream_SeekTime(u8 *stream, void *, u32 *out, u32 ms);

}  // extern "C"
