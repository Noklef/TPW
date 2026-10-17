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

// 0x592220 (thiscall; the plane pointer's address, stride, n; pops 0xc; the SEH frame left out; name unconfirmed): the forward D4 step on
// the n x n plane (rows stride floats apart), the columns first (an inline step, its low-pass sum in filter order), then the rows
// (WaveletCodec_ForwardD4Row), each through stack arrays {data, size, max, grow by}: the row-in one from Mem_Alloc and Dwords_Zero, the
// others from IntArray_SetSize (row out, column in, column out); freed in the order column out, row out, column in, row in.
void __fastcall WaveletCodec_ForwardD4(u8 *codec, void *, float *const *plane, u32 stride, u32 n);

// 0x591d70 (thiscall on the codec; width, height, pixels, destination, the packed size's address, quantiser; pops 0x18; name unconfirmed):
// the grey encoder. The side is the larger of the powers of two above width - 1 and height - 1 (at least 8); the plane at +0x2c becomes
// side x side floats (IntArray_SetSize's steps, inline there), zeroed, the bytes going in row by row; then the forward D4, each value times
// 1 / q (rounded to a float), and the coefficients (__ftol, the low word) written to the stream at 0xa067f8 as WaveletCodec_PutCoefficient
// does; Buffer_Compress (stream, dst, its length, +0x4c) gives the packed size, stored and returned.
u32 __fastcall WaveletCodec_EncodeGrey(u8 *codec, void *, u32 width, u32 height, const u8 *pixels, void *dst, u32 *outSize, float q);

// 0x591ab0 (thiscall on the codec interface, the codec 0x54 below; width, height, colour flag (its low byte), pixels, destination, the packed
// size's address, the Y, Cb and Cr quantisers; pops 0x24): without the flag WaveletCodec_EncodeGrey with the Y quantiser. Else the three
// planes (+0xc, +0x1c, +0x2c) are sized side x side + 1 (sic) and zeroed, the pixels unpacked, turned to YUV, Cb and Cr halved, the
// forward D4 run on Y (side) and Cb, Cr (side / 2), the planes scaled by 1 / q, and Y's side x side then Cb's and Cr's quarter of that
// written with WaveletCodec_PutCoefficient before Buffer_Compress, as in the grey one.
u32 __fastcall WaveletCodec_Encode(u8 *iface, void *, u32 width, u32 height, u32 colour, const u8 *pixels, void *dst, u32 *outSize, float qY, float qCb,
                                   float qCr);

// 0x592b80 (thiscall, this unused; the cursor's address, destination, count, quantiser; pops 0x10; name unconfirmed): the decoders'
// coefficient reader. Once (flag 0xf75ad4) the tables at 0xfb2be0 (each byte as s8), 0xfb2fe0 (as u8) and 0xfb27e0 (s8 times 256.0) are
// built; then count coefficients (a do-while: 0 runs 2^32 times, sic) go to dst times q: a byte other than 0x80 as its s8, 0x80 then a
// little-endian s16 (low byte unsigned plus the high byte's s8 times 256). The cursor moves past them.
void __fastcall WaveletCodec_ReadCoefficients(u8 *codec, void *, const u8 **cursor, float *dst, u32 count, float q);

// 0x592c50 (thiscall, this unused; source, destination, n; pops 0xc; name unconfirmed): one inverse D4 step along a row: the n / 2 low
// values at src[0..] and the high ones at src[n / 2..] interleave into dst[2..] pair by pair, then the wrapped pair into dst[0], dst[1].
// For n >= 6 the original's software-pipelined loop (loads placed as there; its last odd value summed in another order, sic), below
// that a plain loop with another summing order.
void __fastcall WaveletCodec_InverseD4Row(u8 *codec, void *, const float *src, float *dst, s32 n);

// 0x5933a0 (thiscall, this unused; the source and destination plane pointers' addresses, width, n; pops 0x10; name unconfirmed): the
// inverse D4 step down the columns of n x n planes (rows width floats apart): n / 2 - 1 row pairs (a do-while: 0 runs 2^32 times, sic),
// each over n columns with InverseD4Row's pipelined loop, then the wrapped first two rows.
void __fastcall WaveletCodec_InverseD4Columns(u8 *codec, void *, float *const *srcPlane, float *const *dstPlane, u32 width, u32 n);

// 0x592e60 (thiscall on the codec; width, height, packed source, pixels, quantiser; pops 0x14; the SEH frame left out; name unconfirmed):
// the grey decoder. The side as in WaveletCodec_EncodeGrey; Buffer_Decompress (source, the stream at 0xa067f8, +0x4c); the plane at
// +0x2c sized side x side (IntArray_SetSize's steps, inline there); a stack array of side floats (IntArray_SetSize) takes each row's
// coefficients (WaveletCodec_ReadCoefficients, inline there) and WaveletCodec_InverseD4Row puts the row into the plane at +0x3c (its
// data pointer read once); then WaveletCodec_InverseD4Columns's steps (inline, its first wrapped row summed in another order) from +0x3c
// into +0x2c; then width x height bytes to pixels, rows side floats apart: below 0.0 or a NaN 0, above 255.0 0xff, else __ftol's low
// byte. The stack array is freed (Ptr_FreeIfSet).
void __fastcall WaveletCodec_DecodeGrey(u8 *codec, void *, u32 width, u32 height, const void *src, u8 *pixels, float q);
}
