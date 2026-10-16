// A record copy with its text, a band match and a byte permutation: reimplementation of 0x489350, 0x56ee90 and 0x56f4f0. Named by what they do until the
// classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x489350 (thiscall on the destination; source, pops 4): the eight dwords at +0..+0x1c and the text at +0x24 (up to and including its NUL) of the source
// are copied; the dword at +0x20 (a pointer into the record) = the source's moved by the distance between the two. Returns the destination.
u8 *__fastcall Record_CopyWithText(u8 *dest, void *, const u8 *source);

// 0x56ee90 (thiscall; four bytes b0..b3 as one dword, pops 4): 0 unless the signed dword at +0x128 <= b0, the signed dword at +0x12c >= b1 and b1 <= b0;
// then 1 when for some i from b1 to b0 the signed byte at +0x130 + i is at most b2 and the signed byte at +0x1b0 + i is at least b3 (b2 and b3 as bytes).
u32 __fastcall Bands_AnyMatch(const u8 *object, void *, u32 packed);

// 0x56f4f0 (stdcall; four bytes, flags word, pops 8): nothing unless the word has a bit of 0x78; bit 0x40 swaps bytes 1 and 3; then bit 8 rotates the bytes
// one place to the right (the last comes first), otherwise bit 0x10 swaps the halves, otherwise bit 0x20 rotates one place to the left.
void __stdcall Bytes4_Permute(u8 *bytes, const u16 *flags);

}  // extern "C"
