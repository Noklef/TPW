// The MP2 decoder's polyphase synthesis (LbMpeg): reimplementation of the synthesis filter, 0x6c9270-0x6c957c, its 32-point DCT,
// 0x6c9580-0x6ca573, and the fixed-point filter, 0x6ca580-0x6cc734.
//
// The DCT is the classic decoders' dct64 (as in mpg123): 32 subband samples in, 17 values written to out0 and 16 to out1 at a stride of 16 floats
// (out0[0], out0[16] .. out0[256]; out1[0] .. out1[240]), the halves the synthesis window then reads from its two buffers.
#pragma once

#include "../../../re/types.h"

// The fixed-point filter's callee: LbMpeg_MmxTransform (0x6d62c0; a bench may put fncmp's stand-in there).
struct Mp2SynthEnv {
    void(__cdecl *transform)(s16 *out0, s16 *out1, const s16 *in);
};
// (nullptr: the game's)
void Mp2Synth_SetEnv(const Mp2SynthEnv *env);

extern "C" {

// 0x6c9580 (cdecl): the five butterfly stages over in[0..31] (cosine factors from the table at 0x711684, 31 floats: 16, 8, 4, 2, 1 per stage),
// then the output sums. Intermediates are float locals or x87 registers exactly as the original keeps them (24-bit precision, a register's wide
// exponent until it is stored), so overflowing and tiny values match too; some outputs are copied as bits through an integer register.
void __cdecl Mp2_Dct64(float *out0, float *out1, const float *in);
// 0x6c9270 (thiscall): one granule's 32 samples per channel (+0x244c channels; fraction: 32 floats each) into 16-bit PCM, the channels
// interleaved (out[c], out[c + channels], ...; out 0 runs only the DCTs). The ring position (+0x243d) steps back one (mod 16) first; each
// channel's two buffers of 0x120 floats (channel 0's at +0x904, channel 1's at +4) take the DCT at it, then the window (0x70bd48, 16 taps per
// sample) runs over one of them: 16 samples of alternating signs, one of the even taps and 15 mirrored and negated, each rounded as fistp does
// (to nearest; out of range 0x80000000) and clipped to -32768..32767. With the dword 0xfb2708 set it makes 8, 1 and 7 at twice the strides.
// The sums keep the original's association (four partial sums per sample).
void __fastcall Mp2Decoder_Synth(u8 *decoder, void *, s16 *out, const float *fraction);
// 0x6d62c0 (cdecl): the fixed-point DCT (MMX). in's 32 halfwords fold into 16 sums in[j] + in[31 - j] and 16 differences in[j] - in[31 - j]
// (wrapping); each of 16 rows of Q14 factors (0xfc0200 for the sums, 0xfc0000 for the differences) times them, summed in 32 bits
// (wrapping) and shifted right 14, gives one halfword. The sums' rows 0-8 go to out0[256 - 32 k] (row 8 to out1[0] too), 9-15 to
// out1[32 (k - 8)]; the differences' rows 0-7 to out0[240 - 32 k], 8-15 to out1[16 + 32 (k - 8)].
void __cdecl LbMpeg_MmxTransform(s16 *out0, s16 *out1, const s16 *in);
// 0x6ca580 (thiscall): the same in 16-bit fixed point (the MMX code), fraction 32 halfwords a channel (0x40 bytes apart, as
// Mp2Decoder_DequantizeFixed leaves them). Each channel's two buffers of 0x120 halfwords sit at +4 + 0x480 c (channel 0 first) and take the
// transform; the window is 16-bit and already signed (0x70c9e8 for the first 16 samples and the even taps, 0x70c5e8 for the 15 mirrored, each
// less twice the ring position), so a sample is the sum of its 16 products in 32 bits (wrapping), shifted right 13 and saturated; the middle one
// sums the 8 even taps' products with their low halfwords cleared. The output stride is 2 samples only when there are exactly two channels.
void __fastcall Mp2Decoder_SynthFixed(u8 *decoder, void *, s16 *out, const s16 *fraction);

}  // extern "C"
