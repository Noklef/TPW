// The C++ library's money_get (MSVC6): _Getmfld, which reads a money field off a stream by the facet's pattern. An iterator is { streambuf, got: whether value holds the next character, value }.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"

extern "C" {

// 0x6a7920 (pops 0x14; this not read): money_get<char>::_Getmfld: over the negative pattern (slot +0x24) the symbol, the sign, the digits and the spaces. out gets the digits, with '-' in front when the second sign matched, or empty when the field was bad. first moves.
// out's allocator byte comes from a string on the stack that never set it. Returns out.
StdString *__fastcall MoneyGet_Getmfld(void *self, void *, StdString *out, u8 *first, u8 *last, u8 international, u8 *ios);
}
