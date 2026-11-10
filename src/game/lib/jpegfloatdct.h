// libjpeg's float DCTs, the exe's x87 copies (re/libmap.csv): hand ports with X87Reg in the listings' order, where the libraries'
// own builds (tpw_jpeg6a, tpw_jpeg6b: SSE rounding, and _ftol's low half of a 64-bit FISTP where they give 0x80000000) differ. The
// libraries' copies stay built and unhooked; kExeMethods maps them to these addresses, so a method call lands here.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x657110 jpeg_fdct_float (jpeg-6a jfdctflt.c, cdecl (data)): the AA&N forward DCT of 64 floats in place, rows then columns.
void __cdecl Jpeg6a_FdctFloat(float *data);

// 0x653d32 forward_DCT_float (jpeg-6a jcdctmgr.c, cdecl (cinfo, compptr, sample_data, coef_blocks, start_row, start_col,
// num_blocks)): each block's samples less 128 as floats, the controller's (cinfo +0x160) do_float_dct (+0x1c) on them, then each
// times the component's (quant_tbl_no +0x10) float divisors (+0x20) plus 16384.5, __ftol'd, less 16384, as a coefficient.
void __cdecl Jpeg6a_ForwardDctFloat(u8 *cinfo, u8 *compptr, u8 **sample_data, s16 *coef_blocks, u32 start_row, u32 start_col,
                                    u32 num_blocks);

}  // extern "C"
