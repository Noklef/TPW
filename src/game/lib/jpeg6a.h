// IJG libjpeg 6a as the game links it, the compressor: the exe's copies (0x64c5f0..0x657cb4, re/libmap.csv) replaced by the library
// built from third_party/jpeg-6a (tpw_jpeg6a), unmodified. See docs/porting-libraries.md.
//
// tpw_jpeg6a renames the library's external names jpeg6a_* (third_party/config/jpeg6a_prefix.h, force-included into it) so it links
// next to 6b, which keeps its own; the hook rows use those names. The declarations use the plain types the library's typedefs name
// (JSAMPARRAY: unsigned char **, JDIMENSION: unsigned int, 8-bit DCTELEM: int), so this header pulls in none of libjpeg's macros. All
// are cdecl, as in the exe (an unoptimised build: each function is its own, with a frame).
// The structs are named by tags of their own, jpeg6a_*: 6b's jpeg6b.h names the same tags (jpeg_common_struct, ...), laid out
// differently, and jpeg6a.cpp renames 6a's jpeglib.h's tags to these (and the untagged JQUANT_TBL / JHUFF_TBL are given these names).
#pragma once

#include <cstddef>

extern "C" {

struct jpeg6a_common_struct;    // jpeg_common_struct: err +0, mem +4, progress +8, is_decompressor +0xc, global_state +0x10
struct jpeg6a_compress_struct;  // jpeg_compress_struct, 0x168 bytes (6a's boolean an int)
struct jpeg6a_quant_tbl;        // JQUANT_TBL (untagged), 0x84 bytes (sent_table at +0x80)
struct jpeg6a_huff_tbl;         // JHUFF_TBL (untagged), 0x118 bytes (sent_table at +0x114)

// jcomapi.c: what compression and decompression share.
// 0x64f3c0: the image pool freed (mem->free_pool of pool 1), global_state back to the start (DSTATE_START 200 for a decompressor,
// else CSTATE_START 100).
void __cdecl jpeg6a_jpeg_abort(jpeg6a_common_struct *cinfo);
// 0x64f40c: mem->self_destruct when there is a manager, then mem and global_state zeroed.
void __cdecl jpeg6a_jpeg_destroy(jpeg6a_common_struct *cinfo);
// 0x64f43e, 0x64f46f: a permanent JQUANT_TBL / JHUFF_TBL (alloc_small of pool 0), sent_table FALSE.
jpeg6a_quant_tbl *__cdecl jpeg6a_jpeg_alloc_quant_table(jpeg6a_common_struct *cinfo);
jpeg6a_huff_tbl *__cdecl jpeg6a_jpeg_alloc_huff_table(jpeg6a_common_struct *cinfo);

// jcparam.c: the compression parameters.
// 0x64cd90: quant_tbl_ptrs[which_tbl] (jpeg_alloc_quant_table when null) set to basic_table * scale_factor / 100 rounded, clamped
// to 1..32767 (1..255 when force_baseline), sent_table FALSE; JERR_BAD_STATE (0x12) unless global_state is CSTATE_START.
void __cdecl jpeg6a_jpeg_add_quant_table(jpeg6a_compress_struct *cinfo, int which_tbl, const unsigned int *basic_table, int scale_factor,
                                         int force_baseline);
// 0x64ce78: tables 0 and 1 from the standard luminance / chrominance tables at scale_factor percent.
void __cdecl jpeg6a_jpeg_set_linear_quality(jpeg6a_compress_struct *cinfo, int scale_factor, int force_baseline);
// 0x64ceb3: quality clamped to 1..100, then 5000 / quality below 50, else 200 - 2 * quality.
int __cdecl jpeg6a_jpeg_quality_scaling(int quality);
// 0x64cef8: jpeg_set_linear_quality at jpeg_quality_scaling(quality).
void __cdecl jpeg6a_jpeg_set_quality(jpeg6a_compress_struct *cinfo, int quality, int force_baseline);

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
