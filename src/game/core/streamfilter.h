// The sound stream filter (vtable 0x70a678; the names are invented): a sound stream in a chain of streams, +4 the next one downstream (its
// slot 18 answers it: the inner stream), +8 the one upstream, +0xc an id, +0x14 the loops left (9999 for ever). The sound streams' slots:
// 6 the sample rate, 7 the read, 8 a restart, 9 a query (an out and a record), 12 the seek, 13 the position, 14 a length, 15 the format
// set, 16 the bits per sample, 17 the channels, 18 the inner stream. The filter answers most of them with the inner stream's; the
// looping stream (0x70aba8, src/game/core/loopstream) and the stream 0x70abf8 share them.
#pragma once

#include "../../../re/types.h"

extern "C" {

// The inner stream's (slot 18) slot, its arguments passed on and its answer returned:
// 0x6bb130 (slot 6: 6), 0x6bb160 (slot 8: 8), 0x6bb0f0 (slot 13: 13), 0x6bb100 (slot 14: 14), 0x6bb140 (slot 16: 16), 0x6bb150 (slot 17: 17);
u32 __fastcall StreamFilter_SampleRate(u8 *stream);
u32 __fastcall StreamFilter_Restart(u8 *stream);
u32 __fastcall StreamFilter_Position(u8 *stream);
u32 __fastcall StreamFilter_Length(u8 *stream);
u32 __fastcall StreamFilter_Bits(u8 *stream);
u32 __fastcall StreamFilter_Channels(u8 *stream);
// 0x6bb0d0 (slot 12, thiscall (position), pops 4: 12) and 0x6bb110 (slot 15, thiscall (format), pops 4: 15).
u32 __fastcall StreamFilter_Seek(u8 *stream, void *, u32 position);
u32 __fastcall StreamFilter_SetFormat(u8 *stream, void *, u32 format);
// 0x6d4cf0 (slot 9 of 0x70abf8, thiscall (out, record), pops 8): the inner stream's slot 9 (out, record); returns the out.
u32 *__fastcall StreamFilter_Query(u8 *stream, void *, u32 *out, u8 *record);
// 0x6c64f0 (thiscall (record), pops 4; slot 5 of 0x70a678 / 0x70aba8): the inner stream's slot 5 (record), then, when the record's byte
// +0x18 has bit 6, SetThenFlag44_14_6c5110 on it with the loops left (+0x14). Returns what the last call answered.
u32 __fastcall StreamFilter_Describe(u8 *stream, void *, u8 *record);
// 0x6c64a0 (thiscall (out, record), pops 8; slot 9 of 0x70a678 / 0x70aba8): StreamFilter_Query's status (into a local: the record's own
// stack slot); negative: *out = -1. Else, when the record's +0x14 has bit 6, the loops left (+0x14) = the record's +0x44; *out = 0.
// Returns the out.
u32 *__fastcall StreamFilter_QueryLoops(u8 *stream, void *, u32 *out, u8 *record);
// 0x6c6520 (thiscall (buffer, count, done, unused), pops 0x10; slot 7 of 0x70a678): *done = 0, then while it is below the count: the
// next stream's (+4) read (slot 7: buffer + *done, the count left, a local got and 1); its answer 1 takes a loop (9999 stays): none
// left returns 1 (got not added), else the inner stream's (slot 18) slot 8 restarts it. *done += got. Returns 0.
u32 __fastcall StreamFilter_Read(u8 *stream, void *, u8 *buffer, u32 count, u32 *done, u32 unused);

}  // extern "C"
