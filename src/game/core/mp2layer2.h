// The MP2 decoder's layer II sample dequantisation (LbMpeg): reimplementation of its float and fixed-point forms, 0x6ccc20-0x6cd217 and
// 0x6cd220-0x6cde4d (mpg123's II_step_two), and the layer II frame decoder, 0x6cdf50-0x6ce593 (mpg123's do_layer2).
//
// The decoder (this): +0x2418 sblimit (the subbands coded), +0x241c jsbound (those below it coded per channel, the rest joint stereo), +0x2428 the
// frame's bytes, +0x242c the bit position in them, +0x244c the channel count. Bits are read most significant first: the big-endian dword at byte
// pos >> 3 shifted right by (-(pos & 7 + k)) & 31 and masked with the table at 0x711700 ((1 << k) - 1 for k < 24; other data past it).
//
// The arguments: the allocations read for the frame (a signed byte per subband and channel; joint subbands give two, the first unused), the
// output (three granules of two channels of 32 subbands), the scale factors (signed bytes, three per coded subband and channel), which of a
// subband's three to use (the granule group, 0-2) and the allocation table (s16 pairs: bits, d; each subband's table 1 << its first entry's
// bits long, d < 0 for samples read one by one, else d indexes the grouping tables through the pointers at 0x782e00, three entries per code).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ccc20 (thiscall): float samples, the granule g of channel c of subband i at out + 0x100 g + 0x80 c + 4 i. A sample read one by one is
// (bits + d) as an integer times the factor muls[k][scale] (the floats at 0x70ce08, 64 per bit count); grouped samples are copied from that
// table as bits. Joint subbands read channel 0's samples as integers, give channel 1 those times its own factor and then scale channel 0's
// stored floats by channel 0's factor. The subbands from sblimit to 31 are zeroed (both channels when there are two, else channel 0).
void __fastcall Mp2Decoder_Dequantize(u8 *decoder, void *, const s8 *alloc_bits, float *out, const s8 *scale, s32 scale_at, const s16 *table);
// 0x6cd220 (thiscall): the same into 16-bit fixed point: out + 0x80 g + 0x40 c + 2 i, the factors the dwords at 0x70e908, each product shifted
// right by 12. Its joint subbands keep channel 0's (bits + d) as a halfword before scaling it.
void __fastcall Mp2Decoder_DequantizeFixed(u8 *decoder, void *, const s8 *alloc_bits, s16 *out, const s8 *scale, s32 scale_at, const s16 *table);
// 0x6cdf50 (thiscall): one layer II frame into 16-bit PCM (out none: the synthesis only updates its buffers). The allocation table is 4
// with +0x243f set, else the dword at 0x711570 + 4 (16 (2 bitrate index (+0x2446) - channels) + sampling rate (+0x2445)); it gives sblimit
// (+0x2418, from 0x711670) and the table (0x782e28); jsbound (+0x241c) is sblimit, or 4 mode extension (+0x2449) + 4 in joint stereo
// (+0x2448 == 1). The allocations (two channels: two per subband below jsbound, one for both past it; else one per subband), the scale
// factor selections (2 bits each allocated one; the mono form counts only sblimit) and the scale factors (18, 12, 6 or 12 bits for selection
// 0, 1, 2 and any other: abc, aab, aaa, abb) are read on the stack; then 12 groups dequantise and synthesise three granules each (float with
// Mp2Decoder_Dequantize / Synth, or with the dword 0xfb2704 set fixed point with the ...Fixed forms), the output moving on +0x2450 samples a
// granule. The loops count down from their bounds, so a jsbound past sblimit reads on until it faults. Returns 1.
s32 __fastcall Mp2Decoder_DecodeLayer2(u8 *decoder, void *, s16 *pcm);

}  // extern "C"
