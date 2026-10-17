// The wavelet codec's coefficient writer: reimplementation of 0x591d30.
//
// WaveletCodec_Encode (0x591ab0) writes its quantised coefficients with this before compressing
// the stream (the decoders are ported to Python in tools/wct2png.py).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x591d30 (stdcall value, cursor): a coefficient within +-127 as one byte, any other (as s16)
// as 0x80 then the word; the cursor moves past. The cursor is re-read after each store, as the
// original does (the escape's word goes where the first re-read put it).
void __stdcall WaveletCodec_PutCoefficient(u32 value, u8 **cursor);

// 0x597090 (cdecl; count): the next `count` bits, least significant first, from the stream of 16-bit words at [0xbe6800] + [0xe74a40]: when fewer bits than count are held
// ([0xaa67f8]) the next word (the position at [0xbe6800] advanced by 2) goes above them in the accumulator at [0xf74a48] and the held count grows by 16. Shift counts use
// their low 5 bits.
u32 __cdecl Wavelet_ReadBits(u32 count);
}
