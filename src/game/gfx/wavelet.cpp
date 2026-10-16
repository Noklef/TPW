#include "wavelet.h"

extern "C" {

void __stdcall WaveletCodec_PutCoefficient(u32 value, u8 **cursor) {
    s16 v = (s16)value;
    if (v >= -0x7f && v <= 0x7f) {
        **cursor = (u8)v;
        ++*cursor;
        return;
    }
    **cursor = 0x80;
    u8 *word = *cursor + 1;
    *cursor = word;
    *reinterpret_cast<u16 *>(word) = (u16)v;
    *cursor += 2;
}
}
