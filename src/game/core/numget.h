// The C++ library's num_get of char (MSVC6), over a stream buffer iterator passed by value. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// istreambuf_iterator<char>: the buffer (null at the end), whether the character is read yet, the character.
struct NumGetIter {
    u8 *buffer;
    u8 got;
    u8 value;
    u8 pad[2];
};

extern "C" {

// 0x698640 (cdecl): _Getifld: the sign, a base's prefix and the digits read into text (leading zeros dropped, at most 0x1f characters), separators checked against the facet's grouping; text empty if they do not fit it. The answer is the base: by flags & 0xe00, 16 for a 0x prefix, 8 for a leading 0 with no base given. numpunct by NumPut_NumpunctOf.
u32 __cdecl NumGet_Getifld(char *text, NumGetIter *first, NumGetIter *last, u32 flags, u8 **locale);

// 0x695ca0 (pops 0x20): do_get of a bool. With ios flags 0x4000 the input matched against the facet's names (false 0, true 1): state 2 when none is read to its end. Otherwise an integer by _Getifld and strtoul: 0 and 1 only. State 1 at the end of the input. Returns first at out.
u32 *__fastcall NumGet_DoGetBool(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *value);

// 0x696450, 0x6968a0 (pops 0x20): do_get of an unsigned short, an unsigned int: strtoul without the '-', negated after; state 2 when nothing converts, errno is set or a short does not hold it.
u32 *__fastcall NumGet_DoGetUshort(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u16 *value);
u32 *__fastcall NumGet_DoGetUint(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value);

// 0x696ce0, 0x6970e0 (pops 0x20): do_get of a long, an unsigned long: strtol, strtoul of the whole text.
u32 *__fastcall NumGet_DoGetLong(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value);
u32 *__fastcall NumGet_DoGetUlong(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value);

// 0x6981f0 (pops 0x20): do_get of a pointer: hexadecimal whatever the flags; value written unless state has 2 set, from before too.
u32 *__fastcall NumGet_DoGetPointer(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value);

}  // extern "C"
