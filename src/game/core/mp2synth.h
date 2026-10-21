// The MP2 decoder's polyphase synthesis (LbMpeg): reimplementation of its 32-point DCT, 0x6c9580-0x6ca573.
//
// The DCT is the classic decoders' dct64 (as in mpg123): 32 subband samples in, 17 values written to out0 and 16 to out1 at a stride of 16 floats
// (out0[0], out0[16] .. out0[256]; out1[0] .. out1[240]), the halves the synthesis window then reads from its two buffers.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c9580 (cdecl): the five butterfly stages over in[0..31] (cosine factors from the table at 0x711684, 31 floats: 16, 8, 4, 2, 1 per stage),
// then the output sums. Intermediates are float locals or x87 registers exactly as the original keeps them (24-bit precision, a register's wide
// exponent until it is stored), so overflowing and tiny values match too; some outputs are copied as bits through an integer register.
void __cdecl Mp2_Dct64(float *out0, float *out1, const float *in);

}  // extern "C"
