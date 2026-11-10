// libjpeg's float DCTs, the exe's x87 copies (re/libmap.csv): hand ports with X87Reg in the listings' order, where the libraries'
// own builds (tpw_jpeg6a, tpw_jpeg6b: SSE rounding, and _ftol's low half of a 64-bit FISTP where they give 0x80000000) differ. The
// libraries' copies stay built and unhooked; kExeMethods maps them to these addresses, so a method call lands here.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x657110 jpeg_fdct_float (jpeg-6a jfdctflt.c, cdecl (data)): the AA&N forward DCT of 64 floats in place, rows then columns.
void __cdecl Jpeg6a_FdctFloat(float *data);

}  // extern "C"
