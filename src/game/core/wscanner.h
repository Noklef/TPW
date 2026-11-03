// A wide text scanner (subject unconfirmed): an object holding a std::wstring at +4 that it reads on from +0x34 (+0x38 its position),
// set up by 0x5e45d0 (which then asks it for a '/'), with three flag bytes at +0x3d and a context pointer at +0x40. Reimplementation of
// its text setter 0x5e6b30, of the keyword pair's constructor 0x5e73f0 and of three token pushes (0x5e57c0, 0x5e6070, 0x5e6540) onto the
// vector of CountedPairs at +0x50, and of the run counter 0x5e6fc0 (a set of characters at +0x14).
#pragma once

#include "../../../re/types.h"
#include "countedpair.h"
#include "stdwstring.h"

// What the keyword adder calls (the port by default; its bench runs it as a recorder).
struct WKeywordsEnv {
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, ...);   // Charset_FormatWide (0x6acd60)
};
void WKeywords_SetEnv(const WKeywordsEnv *env);

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

// 0x5e49f0 (thiscall (id, handler), pops 8; name invented, unconfirmed): a keyword added: the text of that id (the formatter 0x6acd60 on the table [0x78bac8], 0x80 characters at
// most) made a wstring, the pair of it and the handler (WKeywordPair_Ctor; the value's other three dwords 0), a list node of 0x28 bytes (next +0, prev +4, the wstring copied at +8, the
// value's four dwords at +0x18) put before the first node of the list [[+0x48]], the count +0x4c up by one, the stack pair's wstring destroyed.
void __fastcall WKeywords_Add(u8 *self, void *, u32 id, u32 handler);

// 0x5e47f0 (thiscall; name invented, unconfirmed): the scanner's 18 keyword handlers each added under two text ids: its id and that id + 0x2b.
void __fastcall WKeywords_AddAll(u8 *self);

// 0x5e57c0, 0x5e6070, 0x5e6540 (thiscall; names unconfirmed): a pair {"", L"", 0xc / 0x12 / 0x15} (its strings built empty, their allocator
// bytes stack garbage) inserted once at the end of the vector of CountedPairs [+0x50] (CountedPairVector_InsertN(last, 1)), the pair
// destroyed (its strings hold no block), then Obj_CopyCToMark: the position +0x38 set to the text's length +0xc. Returns what that returns.
u32 __fastcall WScanner_PushToken0c(u8 *scanner);
u32 __fastcall WScanner_PushToken12(u8 *scanner);
u32 __fastcall WScanner_PushToken15(u8 *scanner);

// 0x5e6fc0 (thiscall; name unconfirmed): the run of characters from the position +0x38 that are in the set of characters at +0x14 (a tree
// of word keys, its head at +0x18; std::set::find inlined: Tree16b_LowerBound, then the key compared): for each one found the position
// moves on by one; the run's length when a character is not in the set, or -1 when the position reaches the text's length (+0xc). Each
// character is read as the string's operator[] reads it: past the length, _Nullstr's; else the string frozen first (a counted block, its
// count byte neither 0 nor 0xff, gets its own copy: _Grow(len, false); then the count byte 0xff).
s32 __fastcall WScanner_SpanInSet14(u8 *scanner);
}
