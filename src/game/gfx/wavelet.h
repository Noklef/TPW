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
}
