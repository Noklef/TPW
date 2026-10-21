// The sound library's 4-bit ADPCM decoder of the IMA kind (unconfirmed): the steps precomputed per index and nibble in 0x77c6c0 (the
// index a byte offset 0-0x1600, 64 bytes a step), the index moves in 0x77dd00 (by the nibble's low 3 bits), samples clamped to 16
// bits. Reimplementation of 0x672cb3 and 0x672c10 (hand-written assembly; they keep every register). The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x672cb3 (cdecl (data, out, count, samples, indices); name unconfirmed): count bytes (at least one: the listing tests after the
// first) of stereo samples, the high nibble left and the low one right, each the channel's sample + the step at its index and nibble,
// clamped, written as a left / right pair; the index moved and clamped to 0-0x1600. The channels' samples and indices start from the
// signed halves of *samples / *indices (left low), are kept in the globals 0x77dd20 / 0x77dd24 (samples) and 0x77dd28 / 0x77dd2c
// (indices) as it goes, and are packed back at the end.
void __cdecl ImaAdpcm_DecodeStereo(const u8 *data, s16 *out, s32 count, u32 *samples, u32 *indices);

// 0x672c10 (cdecl (start, data, out, count, sample, index); name unconfirmed): count nibbles (at least one) of mono samples from the
// nibble start on (an even one the byte's high half, an odd one its low half and then the next byte), each the sample + the step at
// the index and nibble, clamped, written; the index moved and clamped to 0-0x1600. The sample starts from *sample's signed lower half,
// the index from the whole *index; both written back whole at the end. No globals.
void __cdecl ImaAdpcm_DecodeMono(s32 start, const u8 *data, s16 *out, s32 count, s32 *sample, s32 *index);
}
