// IJG libjpeg 6b as the game links it: the exe's copies (0x6428b0..0x64bbd0, re/libmap.csv) replaced by the library built from
// third_party/jpeg-6b (tpw_jpeg6b), unmodified. See docs/porting.md, "Library functions".
//
// The hook table points the exe's addresses at these. The library's GLOBAL functions link by their own names. Its METHODDEF and LOCAL
// functions are static (jmorecfg.h defines them so unconditionally): a module whose statics are hooked is compiled through a wrapper
// unit (third_party/wrap/jpeg-6b/<module>.c) that renames each of them <module>_<name> and makes it external, the code unchanged.
// The declarations use the library's types by struct tag or as the plain types its typedefs name (JSAMPARRAY: unsigned char **,
// JCOEFPTR: short *, JDIMENSION: unsigned int), so this header pulls in none of libjpeg's macros. All are cdecl, as in the exe.
#pragma once

#include <cstddef>

extern "C" {

struct jpeg_decompress_struct;  // 0x1b0 bytes (config/jconfig.h's boolean)
struct jpeg6b_component_info;   // jpeg_component_info (an untagged typedef in jpeglib.h), 0x54 bytes

// jutils.c
// 0x6428c0: a rounded up to a multiple of b (b > 0).
long __cdecl jround_up(long a, long b);
// 0x6428e0: num_rows rows of num_cols samples from input_array[source_row...] to output_array[dest_row...].
void __cdecl jcopy_sample_rows(unsigned char **input_array, int source_row, unsigned char **output_array, int dest_row, int num_rows,
                               unsigned int num_cols);
// 0x642930: num_blocks coefficient blocks (64 shorts each) from input_row to output_row.
void __cdecl jcopy_block_row(short (*input_row)[64], short (*output_row)[64], unsigned int num_blocks);
// 0x642960: bytestozero bytes at target cleared.
void __cdecl jzero_far(void *target, std::size_t bytestozero);

// jidctint.c, jidctfst.c, jidctred.c: one block's inverse DCT, dequantized by compptr->dct_table, range-limited through
// cinfo->sample_range_limit into output_buf's rows at output_col (8, 8, 4, 2 and 1 samples square).
// 0x64a5c0 (accurate integer), 0x64abb0 (fast integer), 0x64b5b0, 0x64b970, 0x64bb90 (scaled: 4x4, 2x2, 1x1). The float one
// (jidctflt.c, 0x64b040) stays the exe's: its (INT32) casts past +-2^31 are the low half of __ftol's 64-bit FISTP there and
// 0x80000000 in the library (SSE2's cvttss2si, or __ftol2_sse as x87 code); the game never selects JDCT_FLOAT.
void __cdecl jpeg_idct_islow(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, short *coef_block,
                             unsigned char **output_buf, unsigned int output_col);
void __cdecl jpeg_idct_ifast(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, short *coef_block,
                             unsigned char **output_buf, unsigned int output_col);
void __cdecl jpeg_idct_4x4(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, short *coef_block,
                           unsigned char **output_buf, unsigned int output_col);
void __cdecl jpeg_idct_2x2(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, short *coef_block,
                           unsigned char **output_buf, unsigned int output_col);
void __cdecl jpeg_idct_1x1(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, short *coef_block,
                           unsigned char **output_buf, unsigned int output_col);
}
