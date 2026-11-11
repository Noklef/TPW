// The C++ library's money_get (MSVC6): _Getmfld, which reads a money field off a stream by the facet's pattern. An iterator is { streambuf, got: whether value holds the next character, value }.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"
#include "stdwstring.h"

// istreambuf_iterator<char> by value: { streambuf, got, value }.
struct MoneyGetIter {
    u32 word[2];
};

extern "C" {

// 0x6a7920 (pops 0x14; this not read): money_get<char>::_Getmfld: over the negative pattern (slot +0x24) the symbol, the sign, the digits and the spaces. out gets the digits, with '-' in front when the second sign matched, or empty when the field was bad. first moves.
// out's allocator byte comes from a string on the stack that never set it. Returns out.
StdString *__fastcall MoneyGet_Getmfld(void *self, void *, StdString *out, u8 *first, u8 *last, u8 international, u8 *ios);

// 0x6a7550 (pops 0x24): money_get<char>::do_get of a long double (a double): _Getmfld's digits, state 1 at the end of the input, state 2 when there are none;
// else copied a character at a time (each through operator[], which freezes the digits' block) and given to _Stold: state 2 when nothing converts or errno
// is set. Returns first at out. Iterators by value. (Names invented, unconfirmed.)
u32 *__fastcall MoneyGet_DoGetLongDouble(void *self, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, double *value);

// 0x6a7760 (pops 0x24): do_get of a string: the same, the digits assigned to value (their block shared when it can be).
u32 *__fastcall MoneyGet_DoGetString(void *self, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, StdString *value);

// money_get<wchar_t> (names invented, unconfirmed): the char ones over wide strings and the wide iterator (the same 8 bytes), ctype<wchar_t>'s is() (slot +8), the facets
// moneypunct<wchar_t, intl> and ctype<wchar_t>; '0' and '-' widened by btowc where used (each zero), the digits to _Stold narrowed by wctob. The groups' counts stay a char string.
// 0x6a0900 (pops 0x14) _Getmfld, 0x6a0510 (pops 0x24) a long double, 0x6a0730 (pops 0x24) a string.
StdWString *__fastcall MoneyGetW_Getmfld(void *self, void *, StdWString *out, u8 *first, u8 *last, u8 international, u8 *ios);
u32 *__fastcall MoneyGetW_DoGetLongDouble(void *self, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, double *value);
u32 *__fastcall MoneyGetW_DoGetString(void *self, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, StdWString *value);
}
