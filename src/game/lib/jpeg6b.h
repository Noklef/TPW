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

// jdcolor.c (through wrap/jpeg-6b/jdcolor.c): the output colour space converters, cinfo->cconvert's color_convert methods. Each
// converts num_rows rows from input_buf (one row array per component) at input_row into output_buf's rows, cinfo->output_width
// pixels a row.
// 0x646a30: cconvert's YCbCr -> RGB tables (Cr_r, Cb_b, Cr_g, Cb_g; 256 entries each, through cinfo->mem->alloc_small).
void __cdecl jdcolor_build_ycc_rgb_table(jpeg_decompress_struct *cinfo);
// 0x646b00: YCbCr -> RGB through the tables and cinfo->sample_range_limit.
void __cdecl jdcolor_ycc_rgb_convert(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int input_row,
                                     unsigned char **output_buf, int num_rows);
// 0x646c40: no conversion: the components interleaved (cinfo->num_components a pixel).
void __cdecl jdcolor_null_convert(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int input_row,
                                  unsigned char **output_buf, int num_rows);
// 0x646cd0: grayscale: component 0's rows copied (jcopy_sample_rows).
void __cdecl jdcolor_grayscale_convert(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int input_row,
                                       unsigned char **output_buf, int num_rows);
// 0x646d00: grayscale -> RGB: each sample three times.
void __cdecl jdcolor_gray_rgb_convert(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int input_row,
                                      unsigned char **output_buf, int num_rows);

// jdmerge.c (through wrap/jpeg-6b/jdmerge.c): the merged upsampler, chroma upsampling and YCbCr -> RGB in one pass for h2v1 and h2v2
// sampling. cinfo->upsample is its my_upsampler (upmethod +0xc, the four tables +0x10..+0x1c, spare_row +0x20, spare_full +0x24,
// out_row_width +0x28, rows_to_go +0x2c).
// 0x646f70: the upsampler's YCbCr -> RGB tables (as jdcolor.c's, into cinfo->upsample).
void __cdecl jdmerge_build_ycc_rgb_table(jpeg_decompress_struct *cinfo);
// 0x647040: spare_full cleared, rows_to_go = cinfo->output_height.
void __cdecl jdmerge_start_pass_merged_upsample(jpeg_decompress_struct *cinfo);
// 0x647060: 2:1 vertical: two output rows a row group through upmethod, the second into spare_row when there is room for one only
// (put out on the next call, jcopy_sample_rows).
void __cdecl jdmerge_merged_2v_upsample(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int *in_row_group_ctr,
                                        unsigned int in_row_groups_avail, unsigned char **output_buf, unsigned int *out_row_ctr,
                                        unsigned int out_rows_avail);
// 0x647130: 1:1 vertical: one output row a row group through upmethod.
void __cdecl jdmerge_merged_1v_upsample(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int *in_row_group_ctr,
                                        unsigned int in_row_groups_avail, unsigned char **output_buf, unsigned int *out_row_ctr,
                                        unsigned int out_rows_avail);
// 0x647170: upmethod for h2v1: a row group's Cb and Cr samples shared by two pixels of one row.
void __cdecl jdmerge_h2v1_merged_upsample(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int in_row_group_ctr,
                                          unsigned char **output_buf);
// 0x6472f0: upmethod for h2v2: shared by a 2x2 block (two luma rows, two output rows).
void __cdecl jdmerge_h2v2_merged_upsample(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int in_row_group_ctr,
                                          unsigned char **output_buf);

// jdsample.c (through wrap/jpeg-6b/jdsample.c): the separate upsampler, each component upsampled into color_buf, then colour converted.
// cinfo->upsample is its my_upsampler (color_buf[10] +0xc, methods[10] +0x34, next_row_out +0x5c, rows_to_go +0x60,
// rowgroup_height[10] +0x64, h_expand[10] +0x8c, v_expand[10] +0x96; 0xa0 bytes). The per-component methods take input_data (the
// component's row group) and output_data_ptr (its color_buf entry) and fill cinfo->max_v_samp_factor rows.
// 0x6462e0: next_row_out = max_v_samp_factor (color_buf empty), rows_to_go = output_height.
void __cdecl jdsample_start_pass_upsample(jpeg_decompress_struct *cinfo);
// 0x646300: upsample: each component's method when color_buf is used up, then cinfo->cconvert->color_convert of what fits.
void __cdecl jdsample_sep_upsample(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int *in_row_group_ctr,
                                   unsigned int in_row_groups_avail, unsigned char **output_buf, unsigned int *out_row_ctr,
                                   unsigned int out_rows_avail);
// 0x646400: full size: the input rows themselves.
void __cdecl jdsample_fullsize_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                        unsigned char ***output_data_ptr);
// 0x646410: a component nobody needs: null.
void __cdecl jdsample_noop_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                    unsigned char ***output_data_ptr);
// 0x646420: integral factors h_expand x v_expand by replication (jcopy_sample_rows for the rows).
void __cdecl jdsample_int_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                   unsigned char ***output_data_ptr);
// 0x646540, 0x6465a0: 2:1 horizontal, and 2:1 both ways, by replication.
void __cdecl jdsample_h2v1_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                    unsigned char ***output_data_ptr);
void __cdecl jdsample_h2v2_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                    unsigned char ***output_data_ptr);
// 0x646630, 0x646700: the same by triangular interpolation ("fancy"; h2v2 reads the context rows input_data[-1] and past the group).
void __cdecl jdsample_h2v1_fancy_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                          unsigned char ***output_data_ptr);
void __cdecl jdsample_h2v2_fancy_upsample(jpeg_decompress_struct *cinfo, jpeg6b_component_info *compptr, unsigned char **input_data,
                                          unsigned char ***output_data_ptr);
}
