// The MP2 decoder object (LbMpeg): reimplementation of its open / close / seek (the stream decoder's vtable 0x70af00 and the buffer
// decoder's 0x7118e0, slots 2 and 3, and their opens 0x6d18e0 / 0x6d1990), the buffer clear and output setup after a first header. The
// rewind is Fn_6d1a50 (core/initializers).
//
// The decoder (this) as in mp2frame.h; besides: +0x243c open, +0x2454 granules a frame (4 for layer I, else 1), +0x2458 the frame's bytes
// (as from the bitrate and rate), +0x245c the rest, +0x2460 the rate / 100, +0x246c the samples a frame (768 or 2304 per channel), +0x2474
// those at the output rate (halved at half rate), +0x2414 the frames and +0x240c the samples of the whole, +0x2468 / +0x2464 / +0x2410 the
// rewind's counters.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6cc740 (thiscall): zeroes the synthesis's buffers from +4: 0x240 dwords with the dword 0xfb2704 set (fixed point), else 0x480.
void __fastcall Mp2Decoder_ClearBuffers(u8 *decoder, void *);
// 0x6ccb10 (thiscall): from the parsed header: +0x2460 = rate / 100; layer I: +0x246c 768 channels, +0x2454 4, +0x2458
// bitrate 12000 / rate + 1, +0x245c bitrate 12 % (rate / 100); else +0x246c 2304 channels, +0x2454 1, +0x2458 bitrate 144000 / rate,
// +0x245c (bitrate 144000 % rate) / 100. The ring position +0x243d 1; +0x2474 = +0x246c (halved, as idiv by 2, at half rate, 0xfb2708).
// 32-bit products; idiv faults (sic) on a rate below 100. Returns 0.
s32 __fastcall Mp2Decoder_SetupOutput(u8 *decoder, void *);
// 0x6d1980 (cdecl): a * b / c with the 64-bit product (imul, idiv: faults, sic, on 0 or a quotient past 32 bits).
s32 __cdecl Mp2_MulDiv(s32 a, s32 b, s32 c);
// 0x6cc780 (thiscall; position, pops 4; slot 3 of the stream decoder): the stream's slot 2 (seek) with it, the bit position +0x242c 0;
// returns what the seek returned.
u32 __fastcall Mp2StreamDecoder_Seek(u8 *decoder, void *, u32 position);
// 0x6d1a80 (thiscall; slot 2 of both): if open (+0x243c), not any more (Zero_5d60c0; the empty Debug_Printf left out). Returns 0.
s32 __fastcall Mp2Decoder_Close(u8 *decoder, void *);
// 0x6d18e0 (thiscall; buffer, bytes, pops 8): closes the decoder if open (slot 2), takes the buffer (+0x2428), open; -1 if reading its
// header (slot 5) fails; else clears the buffers, sets the output up and rewinds, then +0x2414 = Mp2_MulDiv(bytes + granules, rate / 100,
// (frame bytes (rate / 100) + rest) granules) and +0x240c that times the samples a frame. Returns 0.
s32 __fastcall Mp2BufferDecoder_Open(u8 *decoder, void *, u32 buffer, s32 bytes);
// 0x6d1990 (thiscall; stream, bytes, pops 8): the same over a stream (+0x2c7c; the frame buffer +0x247c as +0x2428), the counts in the
// other order, then the stream's slot 2 (seek) to 0.
s32 __fastcall Mp2StreamDecoder_Open(u8 *decoder, void *, u32 stream, s32 bytes);
// 0x6c9210 (thiscall; the stream decoder's through the thunk 0x6c8a50): the buffer decoder's vtable 0x7118e0 set, then closed if open.
void __fastcall Mp2Decoder_Dtor(u8 *decoder, void *);
// 0x6c9230 / 0x6c8a20 (thiscall; flags, pops 4; slot 0 of the buffer / stream decoder): destroyed, then (flags bit 0) back to its pool
// (Mp2BufferPool_Delete 0x2478 / Mp2StreamPool_Delete 0x2c80). Returns the decoder.
u8 *__fastcall Mp2BufferDecoder_DeletingDtor(u8 *decoder, void *, u32 flags);
u8 *__fastcall Mp2StreamDecoder_DeletingDtor(u8 *decoder, void *, u32 flags);

}  // extern "C"
