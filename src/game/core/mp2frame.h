// The MP2 decoder's frame headers (LbMpeg): reimplementation of the header parser 0x6cc7b0, the frame readers of the two decoders (the
// stream decoder's vtable 0x70af00 and the buffer decoder's 0x7118e0, slots 4 and 5) and the output setup 0x6ccb10.
//
// The decoder (this) as in mp2layer2.h; the header's fields go to bytes: +0x2442 the layer (4 - bits 17-18: 1-4), +0x2443 the ID bit (19),
// +0x2444 the protection bit, +0x2445 the bitrate index (12-15), +0x2446 the sampling rate's row (0x7114c8: bits 10-11, + 3 for the
// half-rate standard, + 6 for MPEG 2.5), +0x2447 padding, +0x2448 the mode (6-7), +0x2449 the mode extension, +0x244a copyright,
// +0x244b original, +0x243e MPEG 2.5 (bit 20 clear), +0x243f lsf (ID clear, or MPEG 2.5). The stream decoder reads through the stream at
// +0x2c7c (slot 1: read (buffer, bytes)) into its frame buffer +0x247c; the buffer decoder's frames lie in +0x2428 at the bit +0x2430.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6cc7b0 (thiscall; header, pops 4): a frame header (as a big-endian dword read). -1 without the 12-bit sync (nothing written); else the
// fields above, the sampling rate (0x7114c8 + 4 row) to +0x2424 and +0x2408 (halved, as idiv by 2, with the dword 0xfb2708 set), +0x2440 the
// rate bits + 3 unless lsf (not written for MPEG 2.5), the channels +0x244c (2 unless the mode is 3) and +0x2450 the channels times 32
// (16 at half rate); -1 with a bitrate index of 0; else the bitrate +0x2420 (0x711308 + 4 (16 (3 lsf + layer) + index)) and the frame's
// bytes after the header +0x2470: layer 1 (bitrate 12000 / rate + padding) 4 - 4, else bitrate 144000 / rate + padding - 4 (32-bit
// products, idiv: a rate of 0 faults, sic). Returns 0.
s32 __fastcall Mp2Decoder_ParseHeader(u8 *decoder, void *, u32 header);
// 0x6cca00 (thiscall; slot 4 of the stream decoder): reads 4 bytes from the stream (a local), parses them (big-endian); -1 if that fails,
// else reads the frame's +0x2470 bytes into +0x247c, the bit position +0x242c 0, and returns 0.
s32 __fastcall Mp2StreamDecoder_ReadFrame(u8 *decoder, void *);
// 0x6ccac0 (thiscall; slot 5): reads 4 bytes from the stream and returns what parsing them returns.
s32 __fastcall Mp2StreamDecoder_ReadHeader(u8 *decoder, void *);
// 0x6cca60 (thiscall; slot 4 of the buffer decoder): the bit position +0x242c = the next frame's (+0x2430); the dword at that byte
// (+0x2428 + position >> 3, arithmetic) parsed (big-endian); -1 if that fails, else the position moves past the header (+32) and the next
// frame's is that + 8 (+0x2470). Returns 0.
s32 __fastcall Mp2BufferDecoder_NextFrame(u8 *decoder, void *);
// 0x6ccaf0 (thiscall; slot 5): parses the buffer's first dword (big-endian) and returns what that returns.
s32 __fastcall Mp2BufferDecoder_ReadHeader(u8 *decoder, void *);

}  // extern "C"
