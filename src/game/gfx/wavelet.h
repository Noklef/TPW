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

// 0x592060 (thiscall on the codec interface, the full codec 0x54 bytes below it; the version and ten arguments, pops 0x2c): version 0x12 calls the full codec's slot 0 and 0x13 its slot 2
// (+8) with the ten arguments and the callee's result is returned; any other version does nothing (EAX is the version less 0x13).
u32 __fastcall WaveletCodec_Decode(u8 *codec, void *, s32 version, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j);

// 0x5920f0 (thiscall; the version and thirteen arguments, pops 0x38): for a version above 0x12 the full codec's slot 1 (+4) is called with the thirteen arguments and its result returned (EAX is not set otherwise).
u32 __fastcall WaveletCodec_DecodeScaled(u8 *codec, void *, s32 version, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j, u32 k, u32 l, u32 m);

// 0x5919a0 (thiscall on the virtual base, 0x54 into the object; the SEH frame left out; name unconfirmed): the vtables go back to the codec's (+ -0x54 = 0x7026d8, the base's own at the
// virtual base offset in the word at +4 of what +-0x50 points at: 0x7026b8, and the word before it holds that offset - 0x50); then the blocks at +-8, +-0x18, +-0x28, +-0x38 and +-0x48 are
// freed when set (GameHeap_Free).
void __fastcall WaveletCodec_VBaseDtor(u8 *base);

// 0x5925e0 (thiscall; width, height, stride, pixels; pops 0x10; name unconfirmed): WaveletCodec_Encode's first step. height rows of width
// 3-byte pixels, packed one after another, go as floats into the planes at +0x2c (byte 0), +0x1c (byte 1) and +0xc (byte 2), each row
// stride floats below the last. The plane pointers are re-read for each store.
void __fastcall WaveletCodec_UnpackBgr(u8 *codec, void *, u32 width, u32 height, u32 stride, const u8 *src);

// 0x592150 (thiscall; count; pops 4; name unconfirmed): the first count values of the planes at +0x2c (b), +0x1c (g) and +0xc (r) become
// Y, Cb and Cr in place (x87, single precision; the constants at 0x702664..0x702680, written as subtractions of negated ones as there).
void __fastcall WaveletCodec_RgbToYuv(u8 *codec, void *, u32 count);

// 0x5924f0 (thiscall, this unused; source, destination, n; pops 0xc; name unconfirmed): one forward D4 step along a row of n floats
// (n a power of two: indices wrap with n - 1), the filter h0..h3 read from 0xc27208..0xc27214 for each product; the n / 2 low-pass
// values go to dst[0..], the high-pass ones to dst[n / 2..]. Nothing for n <= 0.
void __fastcall WaveletCodec_ForwardD4Row(u8 *codec, void *, const float *src, float *dst, s32 n);

// 0x592670 (thiscall, this unused; the plane pointer's address, n; pops 8; name unconfirmed): the n x n plane is halved in place, each
// 2 x 2 block's average (summed bottom row first) going to the (n / 2) x (n / 2) plane at its start. The plane pointer is re-read for
// each value.
void __fastcall WaveletCodec_HalvePlane(u8 *codec, void *, float *const *plane, u32 n);
}
