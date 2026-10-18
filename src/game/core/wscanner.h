// A wide text scanner (subject unconfirmed): an object holding a std::wstring at +4 that it reads on from +0x34 (+0x38 its position),
// set up by 0x5e45d0 (which then asks it for a '/'), with three flag bytes at +0x3d and a context pointer at +0x40. Reimplementation of
// its text setter 0x5e6b30.
#pragma once

#include "../../../re/types.h"
#include "stdwstring.h"

extern "C" {

// 0x5e6b30 (thiscall (text by value, context, flags), pops 0x18; name unconfirmed): +0x3d / +0x3e = the word at flags, +0x40 = context,
// +0x3f = flags[2]; the wstring at +4 assigned the text (std::wstring::assign(text, 0, npos) inlined, npos read from 0x6fef00: the text's
// block shared when it is whole, not empty and its count byte below 0xfe; else _Grow(n, true), the characters copied and the end set),
// then frozen (when its count byte is neither 0 nor 0xff, _Grow(len, false) for its own copy; the count byte 0xff); +0x38 = 0 and +0x34
// = its characters. The text is destroyed: its count byte down by one, or (0 or 0xff) its block freed.
void __fastcall WScanner_SetText(u8 *scanner, void *, StdWString text, u32 context, const u8 *flags);
}
