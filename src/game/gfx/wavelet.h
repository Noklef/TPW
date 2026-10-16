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
}
