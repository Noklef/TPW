// The MP2 decoder's layer I frame decoder (LbMpeg): reimplementation of 0x6ce5a0-0x6d17a3 (mpg123's do_layer1, its sample steps inlined).
//
// The decoder (this) as in mp2layer2.h: +0x2428 the frame's bytes, +0x242c the bit position (bits most significant first, masked with the
// table at 0x711700), +0x244c the channel count, +0x2450 the samples the output moves on a granule.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ce5a0 (thiscall): one layer I frame (12 granules of 32 subbands) into 16-bit PCM. Returns 1.
//
// Float (the dword 0xfb2704 clear): the allocations (two channels: 8 bits a subband, channel 0's in the high nibble; else 4 bits), a 6-bit
// scale factor for each allocated one in that order, then per granule the samples (allocation + 1 bits each) and
// (sample - 2^allocation + 1) times the float factor at 0x70ce08 row allocation + 1, column scale (fild, fmul), unallocated subbands 0; each
// granule goes through Mp2Decoder_Synth (channel 1 at +0x80) and the output moves on even when it is none.
//
// Fixed point (0xfb2704 set): the allocations are read as whole bytes at the byte the bit position is in (two channels: 32 bytes, channel 0
// in the high nibble; else 16, a subband a nibble), the position moved on 256 or 128 bits, then each subband's scale factors (6 bits, channel
// 0 then 1). All 12 granules are dequantised first ((the dword at 0x711860 + 4 allocation, -(2^allocation - 1), + the sample) times the
// dword at 0x70e908 row allocation + 1, in 32 bits, shifted right 12, as a halfword; channel 1 at +0x40), then synthesised with
// Mp2Decoder_SynthFixed, ending with EMMS.
s32 __fastcall Mp2Decoder_DecodeLayer1(u8 *decoder, void *, s16 *pcm);

}  // extern "C"
