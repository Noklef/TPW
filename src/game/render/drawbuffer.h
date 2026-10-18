// The draw buffer's methods: reimplementation of 0x5821e0, 0x582280, 0x582340, 0x582390, 0x5823f0 and 0x582640-0x5827f0.
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

// 0x5823f0 (thiscall; name unconfirmed): the draw buffer's blocks made, each a tracked block (TrackedGlobal_Alloc, core/gameheap): stream 2
// (+0x20) then stream 1 (+8), each 1 MB of vertices (+0, room +8) and 0x20000 bytes of indices (+0xc, room +0x14 = 0x10000), the used
// counts (+4, +0x10) 0; the batches (+0x44, 0x38000 bytes, +0x48 = 0x1000) and the scratch block (+0x60, 1 MB + 0xf; +0x58 its first
// 16-byte boundary, +0x3c 1 MB above that); then DrawBuffer_Reset (all streams): 1. A stream not got pushes the error (3, 0xb0754000) twice,
// the batches or the scratch block once (EngineErrors_Push), and what was got is freed (a stream that failed, its vertices; stream 2
// failing returns at once; else both streams as 0x582340, the batches and the scratch block, set or not): 0.
u32 __fastcall DrawBuffer_Create(u8 *buffer);

// 0x582340 (thiscall on a stream's record; name unconfirmed): its indices (+0xc) and vertices (+0), when set, freed
// (TrackedGlobal_Free, core/gameheap) and cleared; both rooms (+8, +0x14) = 0. Returns 1.
u32 __fastcall DrawStream_Free(u8 *stream);

// 0x5821e0 (cdecl; name unconfirmed): with a draw buffer at [0x8bcbc4] already, 0. Else a new one (0x64 bytes, GameHeap_Alloc; its streams'
// records, +0 and +0x44-+0x60 cleared, the rest as it comes) goes there; none got: 0. DrawBuffer_Create on it: 1, or on a failure the
// buffer freed, [0x8bcbc4] cleared, the error (3, 0xb0754000) pushed (EngineErrors_Push) once more and 0.
u32 __cdecl DrawBuffer_Init();

// 0x582280 (cdecl; name unconfirmed): 0 without a draw buffer ([0x8bcbc4]). Else stream 2's, then stream 1's blocks freed as
// 0x582340 does (inlined), the batches (+0x44) and, when set, the scratch block (+0x60; +0x58 cleared with it) freed and cleared
// (TrackedGlobal_Free), the buffer freed (GameHeap_Free) and [0x8bcbc4] cleared. Returns 1.
u32 __cdecl DrawBuffer_Destroy();

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

// 0x565960 (thiscall on the object DrawBatch_Open keeps its batch in (+0x20; vertex size +0x1c, vertex end +0x28, index end +0x2c);
// vertex count pointer, indices, count, mask; pops 16; name unconfirmed): 0 when the batch's vertices (+0x28, its count +0x20 plus *the
// vertex count, of the object's size) or indices (+0x2c, its count +0x24 plus count) pass the ends (unsigned). The clip codes of the
// vertices from the batch's +0x28 (*the vertex count of them, stride 0x20; ClipCodes_Compute, the caller's ECX [0x879a20]) into a 0x400-byte
// local (an index past it reads the original's stack); 0 when all share a code. Then the triangles of the indices (count read 3 at a time)
// written from the batch's +0x2c on: one whose codes share a bit outside the mask is skipped; the list stops once the write position passes
// the index end - 0x18 or the vertices (32 bytes each, *the vertex count re-read) the vertex end - 0x140. A triangle with a code outside the
// mask is clipped into a fan (ClipPoly_TriangleFan with those codes, stride 0x20; it may add vertices). Returns the indices written.
s32 __fastcall DrawBatch_AddClippedTriangles(u8 *object, void *, u32 *vertexCount, const u16 *indices, s32 count, u32 mask);

// 0x565bb0 (thiscall on the same object; vertices, count, stride, codes, flags; pops 20; name unconfirmed): nothing unless there is room for
// 10 more vertices (of the object's size) and 0x30 bytes of indices. codes is {and, or} of the vertices' clip codes, or 0 to have them
// computed (ClipCodes_Compute, the caller's ECX [0x879a20]; into an 8-byte local, sic: more vertices run over the original's return
// address). An and with a bit: nothing. With an or the polygon is clipped into the batch's next vertices (ClipPoly_Vertices with the
// vertices as its second buffer; nothing more under 3), else copied there. The batch's vertex count rises by the count and its index count
// by 3 (count - 2); then for 3-8 vertices the fan's triangles from its index end, the last first: (first, first + k, first + k + 1), with
// flag 2 (first, first + k + 1, first + k). Returns 1.
u32 __fastcall DrawBatch_AddPolygon(u8 *object, void *, u8 *vertices, u32 count, u32 stride, const u8 *codes, u32 flags);

}
