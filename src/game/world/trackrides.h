// The track rides' module (TRAK, saved by 0x5428e0, loaded by 0x543560): reimplementation of
// its chunk write 0x543490, its stream read 0x54bbb0, the free record scan 0x549b50 and the
// handle lookup 0x549d30. The module is loaded while 0x877b58 holds 'GFEJ' (0x4a454647); its
// rides are 0xd0-byte records from [0x877b60], named by the handle index | serial (+0) << 8.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x543490 (chunk, buffer, count): Stream_Write of buffer to the chunk's stream (+4); 0 when
// its result is negative (never: it is 0 or 1), else the chunk's count (+0x10) grows by count
// and the bytes written are returned. The chunk (0x18 bytes) starts with a 12-byte header at
// +0xc: version 1, the count, 0.
s32 __cdecl TrakChunk_Write(u8 *chunk, const void *buffer, s32 count);

// 0x54bbb0 (&result, stream, buffer, count, &done): Stream_Read; returns &result.
s32 *__cdecl TrakStream_Read(s32 *result, Stream *stream, void *buffer, s32 count, s32 *done);

// 0x549b50: the first of the 0x100 0xac-byte records at [0x877b68] (none: 0) with bit 0 of +0
// clear, or 0.
u8 *__cdecl TrakPool_FirstFree();

// 0x549d30 (handle): the ride's link (+0x58) +8 when the module is loaded, the handle names
// the ride and the link is set with flag 0x400000 in its +0; else 0. (Not loaded, a handle of
// 0 reads the link from address 0x58 and faults.)
u32 __cdecl TrakRide_LinkedValue(u32 handle);
}
