// The track rides' module (TRAK, saved by 0x5428e0, loaded by 0x543560): reimplementation of
// its chunk write 0x543490, its stream read 0x54bbb0, the free record scan 0x549b50 and the
// handle lookup 0x549d30, and the lookups 0x5452a0 and 0x54ab40, the setters 0x544c00, 0x545100 and 0x54abd0, and the chunk reads 0x544160 and 0x5441f0. The module is loaded while 0x877b58 holds 'GFEJ' (0x4a454647); its
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

// 0x5452a0 (handle): the ride's dword at +0x5c when the handle names it, else 0 (not loaded, a
// handle of 0 reads address 0x5c and faults).
u32 __cdecl TrakRide_Field5c(u32 handle);

// 0x54ab40 (handle): when the module is loaded and the handle names the ride, its first node
// (the list head at +0xc8) goes back on the free list at 0x877b8c and the node's first dword
// is returned; 0 for an empty list, a stale handle or no module.
u32 __cdecl TrakRide_PopNode(u32 handle);

// 0x544c00 (handle): the ride's dword at +0x54 = 1 when the module is loaded and the handle names the ride. (Not loaded, a handle of 0 writes to address 0x54 and faults.)
void __cdecl TrakRide_MarkWornOut(u32 handle);

// 0x545100 (handle, value): the same for the dword at +4 = value.
void __cdecl TrakRide_SetField4(u32 handle, u32 value);

// 0x54abd0 (handle, value): when the module is loaded, the handle names the ride and the free list at 0x877b8c has a node: the node (+0x10 the next free one) is taken
// off it, put at the head of the ride's list at +0xc8, given the handle at +4 and the value at +0.
void __cdecl TrakRide_PushNode(u32 handle, u32 value);

// 0x544160 (chunk, buffer, count): count bytes (at most those left in the chunk: +0x10 - +8) are read from the chunk's stream (+4) into the buffer; the chunk's position (+8)
// advances by what was read and the rest of the count is zeroed in the buffer. Returns the bytes read (0 when the read failed).
u32 __cdecl TrakChunk_Read(u8 *chunk, u8 *buffer, u32 count);

// 0x5441f0 (chunk): the same for one dword: its value when all four bytes were read, otherwise 0.
u32 __cdecl TrakChunk_ReadDword(u8 *chunk);

// 0x544a10 (handle): closes the ride when the module is loaded, the handle names it and its dword at +0x50 is not 0: that is cleared; the list at +0xc8 gets the list at +0xc4
// appended (or becomes it when empty) and +0xc4 is cleared; for the five ride kinds (the dword at +0 is -14, -11, -6, -3 or -1) the pool records at [0x877b68] (0xac bytes
// each, as many used ones - bit 0 of +0 - as the dword at 0x877b6c counts) that belong to the ride (+0x9c) get +0x88 cleared and bits 0x10020 set, 0x405e cleared in +0.
void __cdecl TrakRide_Close(u32 handle);
}
