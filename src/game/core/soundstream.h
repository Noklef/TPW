// The sound stream (vtable 0x70a778; the names are invented), the base of the stream filter (src/game/core/streamfilter) and the
// sound sources: a stream in a chain, +4 the next stream downstream, +8 the one upstream, +0xc an id. Its slots: 0 the destructor, 1 the
// next set, 2 the close, 3 the unlink, 4 the find by id, 5 a description, 6 the sample rate, 9 a query (out, record), 11 a send; the
// chain's slots pass on to the next stream. Reimplementation of 0x6bab30, 0x6ba9a0, 0x6c61e0-0x6c6300, 0x6c6d70 and 0x6ba460; the
// stream 0x70a6c8's and the notify stream's (0x70a748) slots.
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
// 0x6c8e80 (thiscall (out, record), pops 8; name invented): SoundStream_Query (its out the record's own stack slot); a negative answer
// *out = -1; else, when the record's flags +0x14 have 0x1000, +0x10 = the record's +0x48 (the user value), and *out = 0. Returns out.
u32 *__fastcall SoundStream_QueryUser(u8 *stream, void *, u32 *out, u8 *record);
// 0x6c8ed0 (thiscall (record), pops 4; slot 5 of 0x70a748; name invented): when the record's +0x18 has 0x1000, its +0x48 = +0x10 and
// its flags +0x14 |= 0x1000 (SetThenFlag48_14_6c5090).
void __fastcall SoundStream_DescribeUser(u8 *stream, void *, u8 *record);
// 0x6c7460 (thiscall (out, record), pops 8; slot 9 of 0x70a6c8): SoundStream_QueryUser (its out the record's own stack slot); a
// negative answer *out = -1; else, with the record's flag 0x800, +0x18 = its +0x4c, and *out = 0. Returns out.
u32 *__fastcall Stream70a6c8_Query(u8 *stream, void *, u32 *out, u8 *record);
// 0x6c74b0 (thiscall (record), pops 4; slot 5 of 0x70a6c8): SoundStream_DescribeUser, then when the record's +0x18 has 0x800, its
// +0x4c = +0x18 (SetThenFlag4c_14_6c50f0).
void __fastcall Stream70a6c8_Describe(u8 *stream, void *, u8 *record);
// 0x6c75d0 (thiscall; the read's (slot 7) helper; name invented): the next (+4) taken and cleared, the callback object's (+0x18) slot 0
// (+0xc, +0x10: thiscall, pops 8), then the old next's slot 2 unless null. Returns whether +4 is still 0 (the callback set none).
u32 __fastcall Stream70a6c8_NextSource(u8 *stream);

// The notify stream (vtable 0x70a748, 0x1c bytes from its pool; the names are invented): a sound stream whose +0x10 is the record's
// user value (+0x48) and +0x18 its callback object (+0x50), told once at the end (slot 11).
// 0x6ba9b0 (thiscall, this unused (record), pops 4): without the record's flag 0x2000 (+0x14) 0; else the flag cleared and a new stream
// (NotifyStreamPool_New, 0x1c): +4 / +8 / +0xc 0, +0x10 the record's +0x48, +0x18 its +0x50, the vtable (+0x14 left); 0 when the
// pool fails.
u8 *__fastcall NotifyStream_Create(void *, void *, u8 *record);
// 0x6bac10 (thiscall (flags), pops 4; slot 0): the base's vtable (Vtbl70a778_Set_6c7090), then, when flags bit 0, the pool's delete
// (NotifyStreamPool_Delete, 0x1c). Returns the stream.
u8 *__fastcall NotifyStream_Dtor(u8 *stream, void *, u32 flags);
// 0x6bab90 (thiscall (buffer, count, done, flag), pops 0x10; slot 7): the next's read (its slot 7) with them, or 0 without a next.
u32 __fastcall NotifyStream_Read(u8 *stream, void *, u32 buffer, u32 count, u32 done, u32 flag);
// 0x6c8e00 (thiscall (out, record), pops 8; slot 9): SoundStream_QueryUser (its out the record's own stack slot); a negative answer
// *out = -1; else, with the record's flag 0x2000, +0x18 = its +0x50, and *out = 0. Returns out.
u32 *__fastcall NotifyStream_Query(u8 *stream, void *, u32 *out, u8 *record);
// 0x6c8e50 (thiscall (value), pops 4; slot 11): with a callback (+0x18), its slot 1 (+0xc, +0x10, the value's low byte), then +0x18 = 0.
void __fastcall NotifyStream_Send(u8 *stream, void *, u32 value);
// 0x6c6d70 (thiscall (out, ms), pops 8): without a next stream (+4) *out = -1. Else the byte offset of the time: the sample rate (slot 6)
// times (ms times 0.001f) (x87 at 24 bits, through a float), doubled when slot 17 answers 2 (AL) and again when slot 16 answers 0x10,
// rounded (fistp); its low two bits, when not 0, added to it (sic). Slot 12 (seek) to it when it is not past the length (slot 14,
// unsigned), else to it modulo the length unless that is 0. *out = 0. Returns out.
u32 *__fastcall SoundStream_SeekTime(u8 *stream, void *, u32 *out, u32 ms);

// 0x6c6f40 (thiscall (format), pops 4; the silence stream's and 0x70ac50's constructors'): the flags byte +0x20 = 0x40 when bit 0 of the
// format's +0x1b is set, else 0x20, or 2 when its +0x1a is 0x10, else or 1; then slot 15 (the rate bits) with the format's word +0x18
// (pushed as a dword, its high half the format's address's).
void __fastcall SoundStream_SetFormat(u8 *stream, void *, const u8 *format);

// 0x6ba460 (stdcall (record, format, stream), pops 0xc; an SEH frame for the news; name invented): a sample's filters chained in front of
// stream (the record's +0x14 flags). Bit 14: a new 0x70abf8 stream (Stream70abf8Pool_New, Init_6c6d30) and, with a stream, it set in
// front of it (slot 1) and SoundStream_SeekTime to the record's +0x54 ms (the status 1 before), then its slot 3 (unlink); a negative
// status returns the stream as it is. Bit 6 with the loops +0x44 above 1 (unsigned): a looping stream (LoopStreamPool_New, Fn_6c6a00:
// the format's loop +0x20 / +0x24, both not 0) or else a plain filter (StreamFilterPool_New, vtable 0x70a678, +4 / +8 / +0xc 0, +0x14
// the loops) chained (SoundStream_Chain) and bit 6 cleared. Bit 21 with +0x58 not 0: a silence stream of +0x58 ms (SilenceStreamPool_New,
// SilenceStream_Ctor with the format) chained. A null new is chained as it is (sic). Returns the chain's front.
u8 *__stdcall SoundStream_BuildChain(u8 *record, const u8 *format, u8 *stream);

}  // extern "C"
