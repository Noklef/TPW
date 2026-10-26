// The MP2 sound samples (LbMpeg's callers): slot 9 of the buffered sample's vtable 0x70ae60 and of the streamed one's 0x70aec0, the
// open that makes the sample's decoder. The sample (this): +0x10 the data (buffered), +0x18 its bytes, +0x2c flags (bit 0: the length
// +0x30 kept), +0x30 the length in samples, +0x14 a position that has to be below it, +0x34 the decoder (null until opened), +0x40 the
// stream (an object, streamed); the sample's slot 6 (+0x18) its rate, slot 15 (+0x3c) a new rate (a word), slot 20 (+0x50) after a
// streamed open. Reimplementation of 0x6c84e0 and 0x6c88c0 (their SEH frames, for the decoder's construction, left out); the names are
// invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c84e0 / 0x6c88c0 (thiscall; out, pops 4): already open: *out 0. Else the decoder from its pool (Mp2BufferPool_New 0x2478 /
// Mp2StreamPool_New 0x2c80; a null block faults, sic), constructed (Init_6c9260; the streamed one's vtable 0x70af00), kept (+0x34) and
// opened: the buffered one through its slot 1 over (+0x10, +0x18), the streamed one by Mp2StreamDecoder_Open over (+0x40, +0x18), then
// the sample's slot 20. A failed open (-1): *out -1. Then the length +0x30 the decoder's samples (+0x240c) unless kept (+0x2c bit 0);
// a sample rate (slot 6, as a float) other than the decoder's (+0x2408): slot 15 with the decoder's rate's low word (the pushed
// register's high word unset, sic), and that word 0 or the float rate over it (x87 at 24 bits, fistp) 0: *out -1; else the length
// divided by that (unsigned), bit 0 cleared. *out 0 when +0x14 is below the length, else -1. Returns out.
u32 *__fastcall Mp2BufferSample_Open(u8 *sample, void *, u32 *out);
u32 *__fastcall Mp2StreamSample_Open(u8 *sample, void *, u32 *out);

// The slots the two MP2 samples (vtables 0x70ae30 and 0x70ae90) share or pair; the names are invented.
// 0x6c83c0 (thiscall (buffer, bytes, done, flag), pops 0x10; slot 7): with +0x1c set *done = 0 and SoundSample_Drained. With the byte
// +0x3c and the decoder's (+0x34) byte +0x2442 2 and +0x2420 0x30: +0x3c 0, bytes zeros, *done = bytes, 0. Else *done = the bytes, at
// most the length (+0x30) less the position (+0x14, unsigned), the sample's slot 23 (buffer, that), the position on by *done (re-read);
// once it is not below the length, slot 19 (flag, *done) answering 0 in its low byte gives 1. Else 0.
u32 __fastcall Mp2Sample_Read(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *done, u32 flag);
// 0x6c8340 (thiscall (out, record), pops 8; slot 9): the sample's slot 21 (the open: a local out) negative: *out = -1. Else, with the
// decoder's byte +0x2442 1 and the record's +0x14 bit 6 or 11, slot 23 (0xfb23d8, 0x300); *out = 0. Returns out.
u32 *__fastcall Mp2Sample_Query(u8 *sample, void *, u32 *out, const u8 *record);
// 0x6c83a0 (thiscall; slot 2): the sample's slot 22 (Mp2Sample_FreeDecoder), then SoundStream_Close.
void __fastcall Mp2Sample_Close(u8 *sample);
// 0x6c84a0 (thiscall; slot 8 of 0x70ae30): the position (+0x14) 0, the decoder reset (Fn_6d1a50), and with its byte +0x2442 1 slot 23
// (0xfb20d0, 0x300).
void __fastcall Mp2BufferSample_Restart(u8 *sample);
// 0x6c8b10 (thiscall; slot 8 of 0x70ae90): the position 0, SubStream_Rewind (+0x40), the decoder reset (Fn_6d1a50).
void __fastcall Mp2StreamSample_Restart(u8 *sample);
// 0x6c8ae0 (thiscall; slot 22): with a decoder (+0x34), its slot 2, then (re-read, when still there) its slot 0 (1), and +0x34 0.
void __fastcall Mp2Sample_FreeDecoder(u8 *sample);

}  // extern "C"
