// The C library's floating point helpers (MSVC6's own names): the control word's two forms, the mantissa of three dwords (the first the most significant, bit 0 its top) and _ld12cvt, from the 12-byte long double to a double or float.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x68bad0: _abstract_cw: the x87 control word in the C library's _control87 form.
u32 __cdecl CRT_abstract_cw(u32 word);

// 0x68bb70: _hw_cw: the C library's form as the x87 control word (the bits it has no place for go).
u32 __cdecl CRT_hw_cw(u32 word);

// 0x68f9d0: __addl: *sum = a + b; 1 for a carry.
s32 __cdecl CRT_addl(u32 a, u32 b, u32 *sum);

// 0x68bc00: _ZeroTail: 1 if the mantissa's bits from `bit` on are all zero.
s32 __cdecl CRT_ZeroTail(const u32 *mantissa, s32 bit);

// 0x68bc70: _IncMan: adds 1 at `bit`, the carry up the dwords; 1 for a carry out of the top.
s32 __cdecl CRT_IncMan(u32 *mantissa, s32 bit);

// 0x68bce0: _RoundMan: keeps `bits` bits: up only past the half; an exact half goes down whatever the last bit. 1 for a carry out of the top.
s32 __cdecl CRT_RoundMan(u32 *mantissa, s32 bits);

// 0x68bd80: _CopyMan.
void __cdecl CRT_CopyMan(u32 *to, const u32 *from);

// 0x68bdb0: _IsZeroMan: 1 if all three dwords are zero.
s32 __cdecl CRT_IsZeroMan(const u32 *mantissa);

// 0x68bdd0: _ShrMan: the mantissa `bits` to the right, zeros in from the top.
void __cdecl CRT_ShrMan(u32 *mantissa, s32 bits);

// 0x68be90: _ld12cvt: the 12-byte number at `from` into `to` by the format (a table of six: highest and lowest exponent, precision, exponent's width, the whole width (0x40 or 0x20, others store nothing), bias).
// 0 done, 1 too big (an infinity), 2 too small (a denormal or zero).
s32 __cdecl CRT_ld12cvt(const u8 *from, u32 *to, const s32 *format);

// 0x68c060 / 0x68c080: _ld12tod / _ld12tof: with the double's table at 0x781728 / the float's at 0x781740.
s32 __cdecl CRT_ld12tod(const u8 *from, u32 *to);
s32 __cdecl CRT_ld12tof(const u8 *from, u32 *to);

// 0x68fa00: __add_12: *x += *y, the twelve bytes as one number.
void __cdecl CRT_add_12(u32 *x, const u32 *y);
// 0x68fa70 / 0x68faa0: __shl_12 / __shr_12: one bit.
void __cdecl CRT_shl_12(u32 *x);
void __cdecl CRT_shr_12(u32 *x);

// 0x68fad0: __mtold12: `length` digit bytes (signed) as a 12-byte number, normalised. Never ends when they are all zero (sic).
void __cdecl CRT_mtold12(const char *digits, u32 length, u32 *out);

// 0x68ff60: __ld12mul: *x *= *y.
void __cdecl CRT_ld12mul(u32 *x, const u32 *y);

// 0x690220: __multtenpow12: *value *= 10^power from the tables at 0x781f70 / 0x7820d0; without keep_low the low word is cleared first.
void __cdecl CRT_multtenpow12(u32 *value, s32 power, u32 keep_low);

// 0x68fbd0: $I10_OUTPUT: ten bytes (low, high, then sign and exponent in a dword, its top half not read) as decimal digits in `out`: s16 decimal point, the sign's character
// (' ' for a zero), the digits' count, the digits and an end. At most 0x15 digits and one to round by; bit 0 of flags takes `digits` after the point. Returns 0 for
// "1#SNAN", "1#IND", "1#INF" and "1#QNAN", else 1. A digit count below 0 rounds on what lay before the digits (sic).
s32 __cdecl CRT_i10_output(u32 low, u32 high, u32 sign_exp, s32 digits, u32 flags, u8 *out);
// 0x68c250: __dtold: the double at x as ten bytes at `to`, returned. A zero loses its sign (sic).
u32 *__cdecl CRT_dtold(u32 *to, const u32 *x);
// 0x68c1c0: _fltout2: a double (its two dwords, so that no x87 load quiets a NaN) as 0x11 digits: flt gets the sign's character, the decimal point, the answer of $I10_OUTPUT and `buf`, which
// gets the digits. Returns flt.
u32 *__cdecl CRT_fltout2(u32 low, u32 high, u32 *flt, char *buf);
// 0x68c120: _fptostr: the digits at flt[3] into buf, cut and rounded to `digits` (below 0: no digits, not rounded), '0's after their end. A carry out of the first digit is
// kept and flt[1] goes up. Returns EAX as it stands: the last digit changed or the end, or the length moved down by one over the unused first place.
u32 __cdecl CRT_fptostr(char *buf, s32 digits, u32 *flt);

// 0x68c440: __strgtold12: reads a decimal number; 1 underflow, 2 overflow, 4 no digits.
u32 __cdecl CRT_strgtold12(u32 *out, const char **end, const char *text, u32 keep_low, s32 scale, s32 decimals, s32 implicit_e);

// 0x6816a0: _fltin2: fills flt (+0 flags: 0x80 overflow, 0x100 underflow, 0x200 no digits; +4 the bytes read; +0x10 the double). Returns flt.
u32 *__cdecl CRT_fltin2(u32 *flt, const char *text, s32 length, s32 scale, s32 decimals);

// 0x6d7710: strtod.
double __cdecl CRT_strtod(const char *text, const char **end);

}  // extern "C"
