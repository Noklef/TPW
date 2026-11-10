// The C library's pow (MSVC6's _CIpow and the helpers its special cases use). Doubles in the C library's register and stack forms.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6855b0: _frnd: x rounded to an integer by the x87 (FRNDINT, the control word's rounding), through a double.
double __cdecl CRT_frnd(double x);

// 0x686f60: _d_inttype: 0 for a denormal or a y that is not an integer, 1 an odd integer, 2 an even one; the half is y / 2 at the
// caller's precision control, so under single precision a big odd y can count as even (sic). A NaN compares equal (unordered: C3).
s32 __cdecl CRT_d_inttype(double y);

// 0x686d30: _powhlp: pow's infinities: y +-infinity by |x| against 1, x +-infinity by y's sign (and, for -infinity, y odd); *result
// from the C library's globals (infinity 0x781198, its NaN 0x7811a0, -0 0x7811b8); 1 for |x| 1 to an infinite power (a domain
// error), else 0. Any other x and y leave *result as it was.
s32 __cdecl CRT_powhlp(double x, double y, double *result);

// 0x67c5d0: _CIpow (x in ST1, y in ST0; the result in ST0), through RegThunk_CRT_CIpow (src/hook/regthunks): x and y as the original
// stores them (doubles at the caller's rounding), io y as the x87 held it (80 bits) in and the result out. The work is under the
// caller's precision control, rounding to nearest, all masked (the caller's word itself if 0x27f). 2^(y log2 x) by FYL2X and F2XM1;
// a negative x needs an integer y (else the indefinite at 0x780f30); x +-0 by y's sign and parity (y < 0: infinity 0x781350 and a
// singularity); NaNs through FADD (a signalling one: type 7); infinities by _powhlp. A result's double underflow or overflow, or an
// inexact one where the caller unmasked precision, goes to _87except unless __fastflag 0xfb1330 is set.
void __cdecl CRT_CIpow(double x, double y, u8 *io);

}
