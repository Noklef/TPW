// The C++ library's money_put<char> (MSVC6): do_put of a number and of a string, _Putmfld, and what they use: moneypunct<char, intl>, streambuf::sputc and the iterator's put.
// An iterator is { failed (its low byte is the flag), buffer }. Bytes are passed in whole dwords and only their low byte is read.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"

extern "C" {

// 0x694ac0 (pops 4): sputc: the byte stored at the put pointer (the pointer behind +0x20, the count behind +0x30) when there is room, answering c & 0xff; otherwise slot 1 (overflow) with c & 0xff.
u32 __fastcall Streambuf_Sputc(u8 *buffer, void *, u32 c);

// 0x694b10 (pops 4): the iterator's put: a null buffer, or sputc answering -1, sets the failed byte. Returns the iterator.
u32 *__fastcall OstreamIter_Put(u32 *it, void *, u32 c);

// 0x69c660, 0x6a1880, 0x6a87f0, 0x6a8810 (pop 4): moneypunct's grouping, positive_sign, curr_symbol, negative_sign: the slots +0xc, +0x14, +0x10, +0x18 with out. Return out.
StdString *__fastcall MoneyPunct_Grouping(u8 *facet, void *, StdString *out);
StdString *__fastcall MoneyPunct_PositiveSign(u8 *facet, void *, StdString *out);
StdString *__fastcall MoneyPunct_CurrSymbol(u8 *facet, void *, StdString *out);
StdString *__fastcall MoneyPunct_NegativeSign(u8 *facet, void *, StdString *out);

// 0x6a8f00 (pops 0x10): _Makpat: the pattern dword in out, from the table at 0x709d18; the default at 0x709d10 for an international facet, a flag other than 0 or 1, or a position outside 0..4. Returns out.
u32 *__fastcall MoneyPunct_MakePattern(const u8 *facet, void *, u32 *out, u8 space, u8 precedes, u8 position);

// 0x6a8cf0 / 0x6a8f70 (pop 4): moneypunct<char, true> / <char, false>'s constructor, on a _Locinfo of "C"; the final vtable 0x709f2c / 0x709f5c. Return the facet.
void *__fastcall MoneyPunctIntl_Ctor(u8 *facet, void *, u32 refs);
void *__fastcall MoneyPunctLocal_Ctor(u8 *facet, void *, u32 refs);

// 0x6a9260 (pops 0x20; this only passed on): do_put of a number: the value as text without its sign (a value of 1e35 or more scaled down, with zeros for that), then _Putmfld.
u32 *__fastcall MoneyPut_DoPutNumber(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, double value);

// 0x6a9480 (pops 0x1c; this only passed on): do_put of a string: a minus sign and the digits (by the ctype facet's table) that follow it, or "0", then _Putmfld.
u32 *__fastcall MoneyPut_DoPutString(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, const StdString *text);

// 0x6a9700 (pops 0x2c; this not read): _Putmfld: the digits in { allocator, ptr, len, res }, a string it owns, with grouping, sign, symbol and padding by the facet's pattern. out gets the iterator. Returns out.
u32 *__fastcall MoneyPut_Putmfld(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, u32 allocator, char *ptr, u32 len, u32 res);
}
