// The C++ library's num_get of char and of wchar_t (MSVC6), over a stream buffer iterator passed by value. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// istreambuf_iterator<char>: the buffer (null at the end), whether the character is read yet, the character.
struct NumGetIter {
    u8 *buffer;
    u8 got;
    u8 value;
    u8 pad[2];
};

// istreambuf_iterator<wchar_t>: the same with a 16-bit unit.
struct WNumGetIter {
    u8 *buffer;
    u8 got;
    u8 pad;
    u16 value;
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

// 0x6974e0, 0x697930, 0x697d90 (pops 0x20): do_get of a float, a double, a long double (a double): _Getffld inlined (the sign, 0x24 digits kept and the rest a power of ten, the facet's decimal
// point written as the C locale's, 'e' with up to 8 digits), then _Stold (float, long double) or _Stod with that power; state 2 when nothing converts or errno is set.
u32 *__fastcall NumGet_DoGetFloat(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, float *value);
u32 *__fastcall NumGet_DoGetDouble(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, double *value);
u32 *__fastcall NumGet_DoGetLongDouble(void *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, double *value);

// 0x69f8b0 (cdecl), 0x69cb60 (pops 0x20): _Getifld and do_get of a bool of wchar_t: the char ones, numpunct<wchar_t> by use_facet inlined; _Getifld widens '0' before the facet,
// and '+', '-', 'x', 'X' after the unit is fetched. The names: falsename and truename wide.
u32 __cdecl NumGetW_Getifld(char *text, WNumGetIter *first, WNumGetIter *last, u32 flags, u8 **locale);
u32 *__fastcall NumGetW_DoGetBool(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, u8 *value);

// num_get of wchar_t (pops 0x20), the same as the char ones above: each literal widened by btowc when it is compared ('0' once, before the facet), each unit narrowed by wctob into text, the separator and
// the decimal point 16-bit, numpunct<wchar_t> by UseFacet_WNumpunct. 0x69d1c0 unsigned short, 0x69d6a0 unsigned int, 0x69db70 long, 0x69e030 unsigned long, 0x69f3c0 pointer, 0x69e4f0 float,
// 0x69e9e0 double, 0x69eed0 long double.
u32 *__fastcall NumGetW_DoGetUshort(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, u16 *value);
u32 *__fastcall NumGetW_DoGetUint(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, u32 *value);
u32 *__fastcall NumGetW_DoGetLong(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, u32 *value);
u32 *__fastcall NumGetW_DoGetUlong(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, u32 *value);
u32 *__fastcall NumGetW_DoGetPointer(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, u32 *value);
u32 *__fastcall NumGetW_DoGetFloat(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, float *value);
u32 *__fastcall NumGetW_DoGetDouble(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, double *value);
u32 *__fastcall NumGetW_DoGetLongDouble(void *self, void *, u32 *out, WNumGetIter first, WNumGetIter last, u8 *ios, u32 *state, double *value);

}  // extern "C"
