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

}  // extern "C"
