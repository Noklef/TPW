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
// 0x6d5200 (thiscall unused, channel, first, scale, factor; pops 0x14; this unused): the channel's +0 first, +4 scale (its bits) and +8 to +0x100
// the scale times factor, factor squared, ... (each the last times factor, x87 floats); +0x104 to +0x674 (the filters' memory) cleared.
void __fastcall VoiceCodec_InitChannel(void *, void *, void *, u8 *channel, u32 first, u32 scale, float factor);

}  // extern "C"
