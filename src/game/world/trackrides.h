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

// The chunk: +0 the stream's position where it starts, +4 the stream, +8 the read position, +0xc the header (version, count, total).

// 0x543400 (size; name unconfirmed): GlobalAlloc(GMEM_ZEROINIT, size) through its import slot; 0 for a size of 0.
void *__cdecl TrakMem_Alloc(u32 size);

// 0x543420 (version, stream; name unconfirmed): a new chunk (GlobalAlloc(GMEM_ZEROINIT, 0x18); none: 0) starting at the stream's position
// (Stream_Tell), the header (version, 0, 0) written to the stream (Stream_Write); unless that result is negative (never) the count grows by
// the header's 12 bytes. Returns the chunk.
u8 *__cdecl TrakChunk_Begin(u32 version, Stream *stream);

// 0x5434d0 (chunk; name unconfirmed): the total (+0x14) becomes the stream's position (Stream_Tell) less the chunk's start; the stream is
// sought back to the start (origin 0), the header written again (as Begin writes it, the count growing by 12 again) and the stream sought to
// where it was; the chunk is freed (GlobalFree). Returns the total, read after the free (sic). (A Debug_Printf is left out.)
u32 __cdecl TrakChunk_End(u8 *chunk);

// 0x544290 (chunk; name unconfirmed): the stream sought to the chunk's start plus its total (origin 0), the chunk freed (GlobalFree).
// Returns the total. (A Debug_Printf is left out.)
u32 __cdecl TrakChunk_Close(u8 *chunk);

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

// 0x544e50 (handle): when the module is loaded and the handle names the ride, its dword at +0x54 is cleared; if it was 2 (the ride was worn out and is fixed now), for the kinds -14
// to -3 and -1 (+0) the pool records of the ride (as in TrakRide_Close) whose dword at +0x2c is not -1 end that particle system (ParticleSystem_End) and get -1 there; for the
// kind -1, when bit 0x4000 of the record is set, +0x10 = 5 and the object the table at 0x7a4610 holds for the record's dword at +8 (when it is not 0) has its layer queue cleared
// and animation bank 5 entry 0 started in mode 1 at speed 1.
void __cdecl TrakRide_Fixed(u32 handle);

// 0x545180 (handle, percent): the ride's performance (+0x1c) is set to the percent (below 0: 0, above 99: 100) and its four values at +0x20..+0x2c are set to the point that
// far (of 100) from the dwords at +0x30..+0x3c to those at +0x40..+0x4c.
void __cdecl TrakRide_SetPerformance(u32 handle, s32 percent);

// 0x545310 (handle, &a, &b, &c, &d): counts the pieces of the ride's chain (the head at +0xbc, +0x24 the next; the kind in the word at +4) twice over and stores (each pointer when not
// 0): a = the pieces of any other kind / 2, b = the longest run of kinds 9 to 13, c = kind 11 pieces / 2, d = the pieces of kind 12 or 13 with +8 zero (sic: twice
// over). Returns all the pieces / 2; 0 when the handle does not name the ride.
s32 __cdecl TrakRide_CountPieces(u32 handle, s32 *others, s32 *longestRun, s32 *kind11, s32 *free);

// 0x54aa80 (handle, value): when the module is loaded, the handle names the ride, its dword at +0x50 is not 0 and the free node list at 0x877b8c has a node: the node is taken off
// it, put at the head of the ride's list at +0xc4, given the value at +0, the handle at +4 and -1 at +8. Returns 1, otherwise 0.
u32 __cdecl TrakRide_AddNextCar(u32 handle, u32 value);

// 0x544840 (handle): opens the ride when the module is loaded and the handle names it. For the kinds -13, -12, -8, -5 (+0) with +0x50 clear: the first piece of the chain at
// +0xa8 with flag 4 is found and the first piece after it without flag 2 (the chain read round) becomes the ride's current one (+0xa4), +0x50 = 2; for the kinds -10, -9, -7,
// -4 the same with +0x50 = 1; the others only get +0x50 = 1. The performance is then set again from +0x1c (TrakRide_SetPerformance). (A chain without a piece with flag 4,
// or with every piece flagged 2, never ends.)
void __cdecl TrakRide_Open(u32 handle);

// 0x545890 (kind, x, y, orientation, handle, value): when the module is loaded, makes a ride of the kind (-1 to -14; a handle names the record, else the first free one of the 0x40
// at [0x877b60] is taken - none: faults) from the template record 0xd0 bytes apart from 0x764178, the ride count [0x877b64] raised, with the orientation at +0xb0, the position
// at +0xb4 / +0xb8 and the value at +4, no chain; then by the template's kind: the five kinds with collision nodes (-14, -11, -6, -3, -1) get one collision node (sizes 0x1600,
// 0x1100, 0x1600, 0x1600 and 0x1200, the node pushed on the list at +0xc0) and eight pieces round it; -13, -12, -8 and -5 one end piece (flag 4); -10, -9, -7 and -4 as many
// pieces (flag 2) as +0x64 says along a line and an end piece; -2 three collision nodes and fourteen pieces. The performance is set from +0x1c (not for -2); returns the
// ride's handle (index | kind << 8), 0 when the module is not loaded.
u32 __cdecl TrakRide_Create(s32 kind, s32 x, s32 y, u32 orientation, u32 handle, u32 value);

// 0x548e80 (cdecl; ride, asleep; name unconfirmed): asleep and the ride's flag 0x200 (+0) clear: the flag is set and the instance [0x7a4610 + 4 * (+8)] leaves the
// renderer (its holder at +8 by Obj_RemoveFromRect over the words at +0xc0 / +0xc4 when the instance's flags (+4) have 0x4000, else Holder_Sleep), so does the instance
// of +0xc when the flags have 0x2002000 and +0xc is not 0, and the parts of the node list at +0x30 (next at +0x10, part at +8; -1 none) are released
// (PartStates_Release on the first instance's holder). Not asleep with the flag set: the flag is cleared and the same in reverse (Holder_AddToGrid / Holder_Wake,
// PartStates_Activate with handle 0). Otherwise nothing.
void __cdecl TrakRide_SetAsleep(u8 *ride, s32 asleep);

// 0x54b2f0 (cdecl; handle, x, y, piece, a, b, c; name unconfirmed): a track section added to the ride when the module is loaded and the
// handle names it; 0 otherwise, for a section of the ride's chain (+0xbc, the next at +0x24) already at (x, y) (after "Duplicate track
// section - IGNORED!" to the empty Debug_Printf), or when the section pool (0x400 records of 0x28 bytes from [0x877b80], bit 0 of +0 in use;
// none: 0) is full. The first free section: +0 = 1 (the count [0x877b84] up), +4 the piece, +8/+0xc/+0x10 a, b, c, +0x14/+0x18 the
// position, +0x20 the ride, +0x24 = 0, appended to the chain; its node list +0x1c is not cleared (sic). By the piece's kind (its low word)
// collision nodes (CollNode_Alloc, put at the head of that list: +0x14 the next, +0x18 the section; w = the ride's +0xc - +8): kinds 1-8,
// in pairs (1 and 5, ...), two at a corner of the 0x1800 square (1: (x + 0x1800, y); 2: (x, y); 3: (x, y + 0x1800); 4: both) of sizes
// w + 0xc00 and 0xc00 - w, both with that corner's flags (0x14, 0x12, 0xa, 0xc) cleared and the second a blocker (0x40); 9 and 12 a box
// (0x20; 12 also 0x100) at the centre of size 0xc20 by w; 10 and 13 the same of w by 0xc20; 11 a circle at the centre of radius
// sqrt(w^2 + 0x900000) (truncated; flag 0x80 for the ride kinds -5, -8, -12 and -13) and the four corners' blockers of 0xc00 - w. Unless the
// piece has 0x10000 a buoy (Buoy_Alloc) at (x + ox, y + 0x1800 - oy) (1: 0xf84, 0xf84; 2: 0x87c, 0xf84; 3: 0x87c, 0x87c; 4: 0xf84, 0x87c;
// 9-13: 0xc00, 0xc00; other kinds y + 0x1800 for both, sic) joins the ride's buoy chain (+0xa8, the next at +0x14; the count +0xac up),
// its +0x10 the ride and +0x18 the first node, whose +0x1c is the buoy (no node, as for the other kinds, faults). A node the pool cannot
// give faults at its flags. Returns 1.
s32 __cdecl TrakRide_AddSection(u32 handle, s32 x, s32 y, u32 piece, u32 a, u32 b, u32 c);
}
