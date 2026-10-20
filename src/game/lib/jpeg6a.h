// IJG libjpeg 6a as the game links it, the compressor: the exe's copies (0x64c5f0..0x657cb4, re/libmap.csv) replaced by the library
// built from third_party/jpeg-6a (tpw_jpeg6a), unmodified. See docs/porting-libraries.md.
//
// tpw_jpeg6a renames the library's external names jpeg6a_* (third_party/config/jpeg6a_prefix.h, force-included into it) so it links
// next to 6b, which keeps its own; the hook rows use those names. The declarations use the plain types the library's typedefs name
// (JSAMPARRAY: unsigned char **, JDIMENSION: unsigned int, 8-bit DCTELEM: int), so this header pulls in none of libjpeg's macros. All
// are cdecl, as in the exe (an unoptimised build: each function is its own, with a frame).
#pragma once

#include <cstddef>

extern "C" {

// jutils.c (the same source as 6b's; the exe's copies use inline REP MOVS / STOS, the library's memcpy / memset)
// 0x6503d0: a / b rounded up (a >= 0, b > 0).
long __cdecl jpeg6a_jdiv_round_up(long a, long b);
// 0x6503e3: a rounded up to a multiple of b (b > 0).
long __cdecl jpeg6a_jround_up(long a, long b);
// 0x650401: num_rows rows of num_cols samples from input_array[source_row...] to output_array[dest_row...].
void __cdecl jpeg6a_jcopy_sample_rows(unsigned char **input_array, int source_row, unsigned char **output_array, int dest_row,
                                      int num_rows, unsigned int num_cols);
// 0x65047f: num_blocks coefficient blocks (64 shorts each) from input_row to output_row (nothing in the exe calls it).
void __cdecl jpeg6a_jcopy_block_row(short (*input_row)[64], short (*output_row)[64], unsigned int num_blocks);
// 0x6504a2: bytestozero bytes at target cleared.
void __cdecl jpeg6a_jzero_far(void *target, std::size_t bytestozero);

// jfdctint.c, jfdctfst.c: one block's forward DCT in place (64 DCTELEMs, rows then columns; the output scaled up by 8 for the
// quantizer). 0x657810 (accurate integer), 0x657480 (fast integer), stored by jinit_forward_dct for JDCT_ISLOW / JDCT_IFAST.
void __cdecl jpeg6a_jpeg_fdct_islow(int *data);
void __cdecl jpeg6a_jpeg_fdct_ifast(int *data);

}  // extern "C"
