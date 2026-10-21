// The movie player's second DCT video (decoded through 0x671940 and the hand-written 0x674ab9-0x675767): its dequantiser, the MPEG-1
// intra matrix (0x705d88) scaled by the quality and by the IDCT's scales (0x705c88, as TgqVideo_SetQuality uses). TQI (EA's MPEG-like
// intra video) is a guess from the matrix and the (107 - quality) / 16 scale. Reimplementation of 0x6710c0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6710c0 (cdecl (out, quality), x87; name unconfirmed): out's 64 dwords: at quality 100 [0x705c88 + i * 4] * 8 (16.16, rounded by
// bit 15 as the IMUL / SHL / SHR / ADC idiom); else the float (107 - quality) * 0.0625 (FILD, FSUBR, FMUL at the game's precision, then
// stored as a float) and each [0x705c88 + i * 4] * __ftol([0x705d88 + i * 4] * that * 65536) (16.16 the same, __ftol's low dword);
// then out[0] [0x705c88] * 8 in both.
void __cdecl TqiVideo_SetQuality(s32 *out, s32 quality);
}
