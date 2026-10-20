// A wide text scanner (subject unconfirmed): an object holding a std::wstring at +4 that it reads on from +0x34 (+0x38 its position),
// set up by 0x5e45d0 (which then asks it for a '/'), with three flag bytes at +0x3d and a context pointer at +0x40. Reimplementation of
// its text setter 0x5e6b30, of the keyword pair's constructor 0x5e73f0 and of three token pushes (0x5e57c0, 0x5e6070, 0x5e6540) onto the
// vector of CountedPairs at +0x50.
#pragma once

#include "../../../re/types.h"
#include "countedpair.h"
#include "stdwstring.h"

extern "C" {

// 0x5e6b30 (thiscall (text by value, context, flags), pops 0x18; name unconfirmed): +0x3d / +0x3e = the word at flags, +0x40 = context,
// +0x3f = flags[2]; the wstring at +4 assigned the text (std::wstring::assign(text, 0, npos) inlined, npos read from 0x6fef00: the text's
// block shared when it is whole, not empty and its count byte below 0xfe; else _Grow(n, true), the characters copied and the end set),
// then frozen (when its count byte is neither 0 nor 0xff, _Grow(len, false) for its own copy; the count byte 0xff); +0x38 = 0 and +0x34
// = its characters. The text is destroyed: its count byte down by one, or (0 or 0xff) its block freed.
void __fastcall WScanner_SetText(u8 *scanner, void *, StdWString text, u32 context, const u8 *flags);

// 0x5e73f0 (thiscall (key by value, a 16-byte value by value), pops 0x20; name unconfirmed): the pair (key, value) that 0x5e49f0 builds for
// its table at +0x48 from a string loaded by id and a handler: the wstring at +0 (its allocator byte the key's, the other three bytes
// kept) made empty and assigned the key as WScanner_SetText assigns (not frozen), +0x10..+0x1c = the value's four dwords, the key
// destroyed. Returns the pair.
u8 *__fastcall WKeywordPair_Ctor(u8 *pair, void *, StdWString key, u32 v0, u32 v1, u32 v2, u32 v3);

// 0x5e57c0, 0x5e6070, 0x5e6540 (thiscall; names unconfirmed): a pair {"", L"", 0xc / 0x12 / 0x15} (its strings built empty, their allocator
// bytes stack garbage) inserted once at the end of the vector of CountedPairs [+0x50] (CountedPairVector_InsertN(last, 1)), the pair
// destroyed (its strings hold no block), then Obj_CopyCToMark: the position +0x38 set to the text's length +0xc. Returns what that returns.
u32 __fastcall WScanner_PushToken0c(u8 *scanner);
u32 __fastcall WScanner_PushToken12(u8 *scanner);
u32 __fastcall WScanner_PushToken15(u8 *scanner);
}
