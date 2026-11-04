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

}  // extern "C"
