// The draw buffer's methods: reimplementation of 0x582390 and 0x582640-0x5827f0.
//
// The draw buffer is the object at [0x8bcbc4] (set up by 0x5823f0). It has two streams, 1 at +8
// and 2 at +0x20, each {vertices, vertex bytes used, vertex room (1 MB), indices, index bytes
// used, index room (0x10000)}: stream n's record is at +24 n - 0x10. The batches (0x38 bytes
// each, 0x1000 of them) are at [+0x44], their count at +0x48; stream 1 takes them from the top
// down (+0x54) and stream 2 from the bottom up (+0x50), +0x4c the one being filled and +0x38 its
// address. A batch's +0x28 / +0x2c are where its vertices and indices start. +0 is the mode of
// the batch begun (its stream in the low 2 bits; 4 adds to the previous batch) and +4 its stream.
// +0x58..+0x3c is a 1 MB scratch area (16-aligned), handed out from +0x5c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x582390 (thiscall; streams, a byte: 1, 2 or both): stream 1 emptied (+0xc, +0x18) with its
// batches starting at the top again (+0x54 = count - 1), else the first dword of each batch above
// +0x54 cleared; stream 2 emptied (+0x24, +0x30, +0x50 = 0); the scratch area emptied.
void __fastcall DrawBuffer_Reset(u8 *buffer, void *, u32 streams);

// 0x582640 (thiscall; n): n * 16 bytes from the scratch area, or 0 when they don't fit.
u32 __fastcall DrawBuffer_Alloc(u8 *buffer, void *, u32 n);

// 0x582670 (thiscall; mode): the stream's next batch begun (made the one filled unless mode has
// 4), cleared, and pointed at the stream's free vertices and indices; returns it.
u8 *__fastcall DrawBuffer_Begin(u8 *buffer, void *, u32 mode);

// 0x5826f0 (thiscall): where the current stream's free vertices start.
u32 __fastcall DrawBuffer_Vertices(const u8 *buffer);

// 0x582710 (thiscall; mode): when the stream's next batch is not the one filled, it becomes a copy
// of it, its indices starting at the current stream's free ones; returns the batch at +0x38.
u8 *__fastcall DrawBuffer_Resume(u8 *buffer, void *, u32 mode);

// 0x582780 (thiscall; vertex end, index end): unless the mode has 4, the current stream's used
// counts moved up to these and its next batch taken (stream 1 down, others up).
void __fastcall DrawBuffer_End(u8 *buffer, void *, u32 vertexEnd, u32 indexEnd);

// 0x5827d0 / 0x5827f0 (thiscall): the end of the current stream's vertex / index room.
u32 __fastcall DrawBuffer_VertexLimit(const u8 *buffer);
u32 __fastcall DrawBuffer_IndexLimit(const u8 *buffer);
// 0x565310 (thiscall; mode, a, kind, b, c, pops 20): the draw buffer at [0x8bcbc4] resumes a batch for the mode (DrawBuffer_Resume) and the object keeps it at +0x20; none gives 0.
// The batch gets b at +4, a at +0x18, the kind at +0x1c, c at +8 and -1 at +0x34; the object's size at +0x1c is 0x18 for kind 0xc4, 0x20 for 0x1c4, 0x28 for 0x2c4, 0x30 for
// 0x3c4 and 0 for anything else. Returns the batch.
u8 *__fastcall DrawBatch_Open(u8 *object, void *, u32 mode, u32 a, u32 kind, u32 b, u32 c);

}
