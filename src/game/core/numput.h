// The C++ library's num_put::do_put of a pointer, of char and of wchar_t (MSVC6).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x694e60 / 0x69bb70 (pops 0x18; this not read): sprintf "%p", then the text padded with fill to the stream's width (+0x14 of ios; on the right when its flags & 0x1c0 are 0x40), through a stream buffer iterator { failed, buffer }, returned at out. Width cleared. A character that cannot be put: failed's low byte set. Wide: each byte by btowc.
u32 *__fastcall NumPut_DoPutPointer(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
u32 *__fastcall NumPutW_DoPutPointer(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);

// 0x695030 / 0x69bd30 (cdecl): num_put::_Fput: text as a float, count bytes: padded to the width (with zeros and count), the sign first when internal; the first decimal point of the C locale in it as numpunct's (facet by the stream's locale, made once in the classic one),
// zeros zeros before an 'e' (its case by the stream's flags & 4) or at the end. Width cleared. Wide: each byte by btowc, btowc('0') also with no zeros.
u32 *__cdecl NumPut_Fput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, const char *text, u32 zeros, u32 count);
u32 *__cdecl NumPutW_Fput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, const char *text, u32 zeros, u32 count);
// 0x695450 / 0x69c190 (cdecl): num_put::_Iput: text as an integer, count bytes: ',' put into it by numpunct's grouping, past a prefix of sign or 0x; padded to the width, after the prefix when internal; each ',' as the facet's separator, none when it is 0. Up to a ',' the text is read to its terminator, not count (sic).
u32 *__cdecl NumPut_Iput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, char *text, u32 count);
u32 *__cdecl NumPutW_Iput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, char *text, u32 count);

// 0x694b80, 0x694c20 / 0x69b890, 0x69b930 (pops 0x18; this not read): do_put of a long, of an unsigned long: sprintf by the stream's flags ("%[+][#]l" and d or u, o, x or X), then _Iput.
u32 *__fastcall NumPut_DoPutLong(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
u32 *__fastcall NumPut_DoPutUlong(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
u32 *__fastcall NumPutW_DoPutLong(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
u32 *__fastcall NumPutW_DoPutUlong(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
// 0x694cc0, 0x694d90 / 0x69b9d0, 0x69baa0 (pops 0x1c; this not read): do_put of a double, of a long double (the same 8 bytes, with 'L'): sprintf by the flags with a precision of at most 36 (6 for none unless fixed; the rest are _Fput's zeros), then _Fput.
u32 *__fastcall NumPut_DoPutDouble(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high);
u32 *__fastcall NumPut_DoPutLongDouble(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high);
u32 *__fastcall NumPutW_DoPutDouble(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high);
u32 *__fastcall NumPutW_DoPutLongDouble(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high);
}
