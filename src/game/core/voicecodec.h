// The voice codec of one of the sample classes (vtable 0x70adc0; 0x6c7cf0 decodes a frame each 0x1b0 samples): an LPC speech codec of
// 432-sample frames, four subframes of 108, order-12 reflection coefficients (quantised by the tables at 0x783ba8 and 0x783be8) turned into
// the synthesis filter by 0x6d61c0, the excitation coded by a 29-state table (0x783ca8, 0x783ea8), one or two channels (+0x14 and +0xd48 in
// the decoder). Its bits are read least significant first through a state {byte pointer, bit buffer, bits in it} by reader objects whose
// vtable is {skip(state, n), read(state, n)}: 0x70adb8 over memory, 0x70ae28 over a 0x2000-byte buffer at +8 refilled through the stream at
// +0x2008 (its slot 1, (buffer, 0x2000)). A read of n bits masks with the table at 0x783b80 (n 0-8: past it, other data, sic) and loads one
// byte when fewer than 8 bits are left. Reimplementation of 0x6d4ee0-0x6d6293; the names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d4ee0 (thiscall state, n; pops 8; this unused): the bit buffer & mask[n], the buffer shifted right by n (n & 31) and n taken from the
// count; when that leaves fewer than 8 (signed), the next byte shifted left by the count (& 31) ORed in, the pointer and count advanced.
u32 __fastcall VoiceBits_Read(void *, void *, u32 *state, u32 n);
// 0x6d4fa0 (thiscall state, n; pops 8): as 0x6d4ee0 without the value; returns the state.
u32 *__fastcall VoiceBits_Skip(void *, void *, u32 *state, u32 n);
// 0x6d4f30 (thiscall state, n; pops 8): as 0x6d4ee0 over the reader's buffer: when the byte loaded was the buffer's last (the pointer
// reaching +0x2008), the stream at +0x2008 refills it (slot 1, (+8, 0x2000)) and the pointer goes back to +8; the count is advanced after.
u32 __fastcall VoiceBitsBuffered_Read(u8 *self, void *, u32 *state, u32 n);
// 0x6d4fe0 (thiscall state, n; pops 8): as 0x6d4f30 without the value (EAX left as it was, or the stream's answer: none).
void __fastcall VoiceBitsBuffered_Skip(u8 *self, void *, u32 *state, u32 n);
// 0x6d5200 (thiscall unused, channel, first, scale, factor; pops 0x14; this unused): the channel's +0 first, +4 scale and +8 to +0x100 (floats, as bits)
// the scale times factor, factor squared, ... (each the last times factor, x87 floats); +0x104 to +0x674 (the filters' memory) cleared.
void __fastcall VoiceCodec_InitChannel(void *, void *, void *, u8 *channel, u32 first, u32 scale, u32 factor);

// 0x6d5040 (thiscall state, coded, out, stride; pops 0x10): one subframe's excitation, floats at out every stride for positions below 108.
// Not coded: by the bit buffer's low two bits 0 (and skipped 1 bit) for 0 or 2, -2 for 1 and 2 for 3 (skipped 2). Coded: a state machine,
// the byte of 0x783ca8 at (the last entry's next state << 8) + the bit buffer's low byte picking an entry of 0x783ea8 {next, bits to
// skip, value}: past 3 its value; 2 and 3 a run of read(6) + 7 zeros (cut to what fits, (108 - position) / stride, signed; none when that
// is not positive, the position then kept); 0 and 1 an escape: 7 plus the 1 bits read one by one up to a 0, signed by one more bit (1 +).
void __fastcall VoiceCodec_Excitation(void *reader, void *, u32 *state, u32 coded, u32 *out, s32 stride);
// 0x6d5260 (thiscall data, state, stereo; pops 0xc): a frame header from memory: the state started at the first byte (pointer past it,
// count 8); +0x10 a stereo bit (read when stereo, else 0), +0xc one bit, then the channels (+0x14, and +0xd48 when stereo) set up by
// 0x6d5200 with 32 - read(4), (read(4) + 1) * 8 and 1.04 - read(6) * 0.001 (x87 floats).
void __fastcall VoiceCodec_ReadHeader(void *reader, void *, const u8 *data, u32 *state, u32 stereo);
// 0x6d5350 (thiscall state, stereo; pops 8): as 0x6d5260 from the reader's buffer, refilled first (the stream's slot 1, (+8, 0x2000)).
void __fastcall VoiceCodec_ReadHeaderBuffered(u8 *reader, void *, u32 *state, u32 stereo);

// 0x6d61c0 (cdecl k, out): the synthesis filter's first 12 terms from the 12 coefficients k: an impulse (1) through a lattice of k[0] to
// k[10] (its state starting as k[0] to k[10], the sum's first term -state[10] * k[11]), each step's output s[j] fed back as the next input
// and out[j] = s[j] - the sum of s[j - 1 - i] * out[i] for i < j (x87, the sums kept on its stack).
void __cdecl VoiceCodec_Impulse(const float *k, float *out);

}  // extern "C"
