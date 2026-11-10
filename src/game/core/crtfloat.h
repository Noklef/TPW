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

// 0x685ee0: _sptype: a double's special kind: +inf 1, -inf 2, quiet NaN 3, signalling NaN 4, else 0.
u32 __cdecl CRT_sptype(u32 low, u32 high);
// 0x685e60: _errcode: the first of the exception flags 0x20, 8, 4, 1 as 5, 1, 2, 3; else twice flag 2.
u32 __cdecl CRT_errcode(u32 flags);
// 0x685ea0: _set_exp: the double with its exponent field exp + 0x3fe (bits past the field fall on the sign, sic: only the sign and the top mantissa bits are masked in).
double __cdecl CRT_set_exp(u32 low, u32 high, s32 exp);
// 0x68c0a0 / 0x68c0e0: _atodbl / _atoflt: the text by _strgtold12 into twelve bytes, then _ld12tod / _ld12tof to out.
s32 __cdecl CRT_atodbl(u32 *out, const char *text);
s32 __cdecl CRT_atoflt(u32 *out, const char *text);
// 0x68b450: _hextodec: a digit (by _isctype, or _pctype when MB_CUR_MAX is 1) as it is; else (c & ~0x20) - 7.
u32 __cdecl CRT_hextodec(u32 c);
// 0x6ac940 (name invented, unconfirmed): strtod, then times [0x70a050] for each of a positive power, or times [0x70a058] for each of a negative one.
double __cdecl CRT_Stod(const char *text, const char **end, s32 power);
double __cdecl CRT_Stold(const char *text, const char **end, s32 power);   // 0x6ac980: Stod's wrapper (name invented, unconfirmed)

// 0x680aa0: shift (_cftoe's): the text moved up by dist with its end; nothing for 0.
void __cdecl CRT_shift(char *text, u32 dist);
// 0x685e00: _set_errno_from_matherr: type 1 errno 0x21, types 2 and 3 errno 0x22, others nothing.
void __cdecl CRT_set_errno_from_matherr(s32 type);
// 0x685e30: _get_fname: the name beside this code in the 27 pairs of [0x7810c0], or 0.
u32 __cdecl CRT_get_fname(u32 code);
// 0x686080: _ctrlfp: the x87 control word's mask bits from value; returns the old word, sign extended.
s32 __cdecl CRT_ctrlfp(u32 value, u32 mask);
// 0x68ba70: _control87: the same in the C library's form of the word (_abstract_cw, _hw_cw); returns the new form.
u32 __cdecl CRT_control87(u32 value, u32 mask);
u32 __cdecl CRT_controlfp(u32 value, u32 mask);   // 0x68bab0: _controlfp: _control87 without the denormal bit in the mask

// 0x6804d0: _setdefaultprecision: _controlfp(_PC_53, _MCW_PC).
void __cdecl CRT_setdefaultprecision();
// 0x6804f0: _ms_p5_test_fdiv: 1 when 4195835 - 4195835 / 3145727 * 3145727 (on the x87) is above 1.0 (the Pentium's FDIV bug), else 0.
s32 __cdecl CRT_ms_p5_test_fdiv();
// 0x680540: _ms_p5_mp_test_fdiv: IsProcessorFeaturePresent(0) from KERNEL32 when it has one, else _ms_p5_test_fdiv.
s32 __cdecl CRT_ms_p5_mp_test_fdiv();
// 0x67a5f0: _FPinit: the conversion pointers (0x67a610), the FDIV answer to [0xfb1334], the default precision, the exception flags cleared.
void __cdecl CRT_FPinit();
// 0x67b3e0: _copysign: the first double with the second's sign, by its words.
double __cdecl CRT_copysign(u32 low, u32 high, u32 other_low, u32 other_high);
void __cdecl CRT_forcdecpt(char *text);         // 0x680570
void __cdecl CRT_cropzeros(char *text);         // 0x6805d0
s32 __cdecl CRT_positive(const double *value);  // 0x680640
double __cdecl CRT_frexp(u32 low, u32 high, s32 *exp);  // 0x685f40
s32 __cdecl CRT_fpclass(u32 low, u32 high);             // 0x67b440
void __cdecl CRT_fassign(s32 is_double, u32 *out, const char *text);  // 0x680660
// 0x680740, 0x6808b0: _cftoe2, _cftof2; number: a STRFLT (sign, decpt, flag, mantissa).
char *__cdecl CRT_cftoe2(char *buffer, s32 decimals, s32 caps, const s32 *number, u8 g_format);
char *__cdecl CRT_cftof2(char *buffer, s32 decimals, const s32 *number, u8 g_format);
// 0x6806c0, 0x680840, 0x680970: _cftoe, _cftof, _cftog: the double at `value` by _fltout2 and _fptostr (after any '-': e with `decimals` + 1 digits past a point's room, f with
// decpt + decimals, g with decimals), then _cftoe2/_cftof2. g: e when the rounded power is under -4 or at least `decimals`, else f; a carry in rounding cuts the last digit.
char *__cdecl CRT_cftoe(const u32 *value, char *buffer, s32 decimals, s32 caps);
char *__cdecl CRT_cftof(const u32 *value, char *buffer, s32 decimals);
char *__cdecl CRT_cftog(const u32 *value, char *buffer, s32 decimals, s32 caps);
// 0x680a30: _cfltcvt: 'e' or 'E': _cftoe; 'f': _cftof; any other: _cftog.
char *__cdecl CRT_cfltcvt(const u32 *value, char *buffer, s32 format, s32 decimals, s32 caps);

}  // extern "C"

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const double, CRT_minus_one_7089b8, 0x007089b8, 0x8);
GAME_DATA(const double, CRT_one_708a70, 0x00708a70, 0x8);
GAME_DATA(const char[0x1c], CRT_text_IsProcessorFeaturePresent_708a78, 0x00708a78, 0x1c);
GAME_DATA(const char[0xc], CRT_text_KERNEL32_708a94, 0x00708a94, 0xc);
GAME_DATA(const double, CRT_zero_708aa0, 0x00708aa0, 0x8);
GAME_DATA(const char[8], CRT_text_exponent_708aa8, 0x00708aa8, 0x8);
GAME_DATA(const double, CRT_zero_708ce0, 0x00708ce0, 0x8);
GAME_DATA(const char[8], CRT_text_QNAN_709730, 0x00709730, 0x8);
GAME_DATA(const char[8], CRT_text_INF_709738, 0x00709738, 0x8);
GAME_DATA(const char[8], CRT_text_IND_709740, 0x00709740, 0x8);
GAME_DATA(const char[8], CRT_text_SNAN_709748, 0x00709748, 0x8);
GAME_DATA(const double, CRT_ten_70a050, 0x0070a050, 0x8);
GAME_DATA(const double, CRT_tenth_70a058, 0x0070a058, 0x8);
GAME_DATA(const double, CRT_zero_711c28, 0x00711c28, 0x8);
GAME_DATA(double, CRT_HUGE, 0x00784c10, 0x8);
GAME_DATA(s32, CRT_adjust_fdiv, 0x00fb1334, 0x4);
