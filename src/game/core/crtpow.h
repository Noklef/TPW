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

}
