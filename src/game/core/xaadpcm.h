// The sound library's 4-bit ADPCM decoders (the PlayStation / CD-XA kind: 28 samples a block, a header byte per channel whose high
// nibble picks the filter, the coefficient pairs at 0x705ec8 / 0x705ed8 (0, 240, 460, 392 / 0, 0, -208, -220, in 256ths; a filter
// above 3 reads past them, sic), and whose low nibble the shift). Samples are 16-bit, stereo out (mono written to both channels).
// Reimplementation of 0x672090 and 0x672210. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x672090 (cdecl (count, position, last, before, data, out); name unconfirmed): for count above 0, (count + 27) / 28 (unsigned) blocks
// of 15 bytes from data + *position * 15 / 28 (signed): *position += 28, the header byte, then 14 bytes, each two samples (the high
// nibble first) of (nibble << 28 >> (shift + 8)) + last * c1 + before * c2 + 0x80 >> 8 (arithmetic; products wrapping), clamped to
// -0x8000..0x7fff, written as a left / right pair through *out. last, before and *out are kept through the pointers whatever the
// count. Returns before (the pointer: the listing's EAX).
s32 *__cdecl XaAdpcm_DecodeMono(s32 count, s32 *position, s32 *last, s32 *before, const u8 *data, s16 **out);

// 0x672210 (cdecl (count, position, last left, before left, last right, before right, data, out); name unconfirmed): the stereo
// blocks of 30 bytes from data + *position * 30 / 28 (signed): two header bytes (the first's high nibble the left filter, its low
// nibble the right one; the second's high nibble the left shift, its low nibble the right one), then 14 pairs of bytes, each byte a
// left sample (high nibble) and a right one (low nibble). Returns the right channel's before as stored (the listing's EAX).
s32 __cdecl XaAdpcm_DecodeStereo(s32 count, s32 *position, s32 *last_left, s32 *before_left, s32 *last_right, s32 *before_right,
                                 const u8 *data, s16 **out);
}
