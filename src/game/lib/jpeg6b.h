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
struct jpeg_common_struct;      // the fields compression and decompression share
struct jpeg6b_quant_tbl;        // JQUANT_TBL (untagged), 0x82 bytes
struct jpeg6b_huff_tbl;         // JHUFF_TBL (untagged), 0x112 bytes
struct jpeg6b_d_derived_tbl;    // jdhuff.h's d_derived_tbl (untagged), 0x590 bytes
struct jpeg6b_bitread_working_state;  // jdhuff.h's bitread_working_state (untagged), 0x14 bytes

// jcomapi.c: what compression and decompression share.
// 0x63dff0: the image pool freed (mem->free_pool of pool 1; nothing without a memory manager), global_state back to the start
// (DSTATE_START with marker_list null, or CSTATE_START).
void __cdecl jpeg_abort(jpeg_common_struct *cinfo);
// 0x63e040: mem->self_destruct when there is a manager, then mem and global_state zeroed.
void __cdecl jpeg_destroy(jpeg_common_struct *cinfo);
// 0x63e070, 0x63e090: a permanent JQUANT_TBL / JHUFF_TBL, sent_table FALSE.
jpeg6b_quant_tbl *__cdecl jpeg_alloc_quant_table(jpeg_common_struct *cinfo);
jpeg6b_huff_tbl *__cdecl jpeg_alloc_huff_table(jpeg_common_struct *cinfo);

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

// jddctmgr.c (through wrap/jpeg-6b/jddctmgr.c): the IDCT manager, cinfo->idct (my_idct_controller: start_pass, inverse_DCT[10],
// cur_method[10] +0x2c). Both are shims (jpeg6b.cpp): they store method addresses.
// 0x645a80: cinfo->idct allocated, a zeroed multiplier table (0x100 bytes) for each component's dct_table, cur_method -1.
void __cdecl jinit_inverse_dct(jpeg_decompress_struct *cinfo);
// 0x645b00: start_pass: each component's IDCT by DCT_scaled_size and cinfo->dct_method, and its multiplier table from its quant_table
// when the method changed (islow: the quantval; ifast: scaled by aanscales; float: by aanscalefactor, x87 in the exe).
void __cdecl jddctmgr_start_pass(jpeg_decompress_struct *cinfo);

// jdhuff.c: the sequential Huffman decoder's exported leaves. The bit reader's get_buffer is a long, bits_left an int.
// 0x6441f0: the derived tables (maxcode, valoffset, lookahead) of dc_huff_tbl_ptrs[tblno] (isDC) or ac_huff_tbl_ptrs[tblno] into
// *pdtbl, allocated when null.
void __cdecl jpeg_make_d_derived_tbl(jpeg_decompress_struct *cinfo, unsigned char isDC, int tblno, jpeg6b_d_derived_tbl **pdtbl);
// 0x6444d0: get_buffer refilled to at least nbits from the data source (FALSE: the source suspended); at a marker, zeros.
unsigned char __cdecl jpeg_fill_bit_buffer(jpeg6b_bitread_working_state *state, long get_buffer, int bits_left, int nbits);
// 0x6445e0: a code of min_bits or more bits decoded through htbl's maxcode / valoffset (-1: suspended; 0 for a bad code).
int __cdecl jpeg_huff_decode(jpeg6b_bitread_working_state *state, long get_buffer, int bits_left, jpeg6b_d_derived_tbl *htbl,
                             int min_bits);
// The decoder itself (through wrap/jpeg-6b/jdhuff.c): cinfo->entropy is its huff_entropy_decoder (pub, bitstate +0xc, last_dc_val[4]
// +0x14, restarts_to_go +0x24, dc / ac_derived_tbls[4] +0x28 / +0x38, dc / ac_cur_tbls[10] +0x48 / +0x70, dc / ac_needed[10] +0x98 /
// +0xa2; 0xac bytes).
// 0x6446d0: the module's init (a shim, jpeg6b.cpp): cinfo->entropy allocated, its derived tables null, with the exe's method
// addresses.
void __cdecl jinit_huff_decoder(jpeg_decompress_struct *cinfo);
// 0x644710: start_pass: a warning unless the scan is sequential (Ss 0, Se 63, Ah = Al = 0); each scan component's derived tables, the
// tables and needs of each block of the MCU, the bit buffer emptied, restarts_to_go = restart_interval.
void __cdecl jdhuff_start_pass_huff_decoder(jpeg_decompress_struct *cinfo);
// 0x644860: decode_mcu: a restart first when one is due, then each block's DC difference (added to its component's last_dc_val) and
// AC coefficients into MCU_data (FALSE: suspended, nothing kept); nothing decoded once the data ran out (insufficient_data).
unsigned char __cdecl jdhuff_decode_mcu(jpeg_decompress_struct *cinfo, short (**MCU_data)[64]);
// 0x644cb0: process_restart: the bit buffer's whole bytes counted as discarded, cinfo->marker->read_restart_marker, the DC
// predictions zeroed, restarts_to_go reset, insufficient_data cleared unless a marker is pending.
unsigned char __cdecl jdhuff_process_restart(jpeg_decompress_struct *cinfo);

// jdphuff.c (through wrap/jpeg-6b/jdphuff.c): the progressive Huffman decoder. cinfo->entropy is its phuff_entropy_decoder (pub,
// bitstate +0xc, EOBRUN +0x14, last_dc_val[4] +0x18, restarts_to_go +0x28, derived_tbls[4] +0x2c, ac_derived_tbl +0x3c; 0x40 bytes).
// 0x644d30: the module's init (a shim, jpeg6b.cpp): cinfo->entropy allocated with the exe's start_pass, and cinfo->coef_bits (64
// ints a component) all -1.
void __cdecl jinit_phuff_decoder(jpeg_decompress_struct *cinfo);
// 0x644db0: start_pass (a shim, jpeg6b.cpp): the scan's progression checked (ERREXIT4 for a bad one, warnings against coef_bits,
// which it updates), the scan's decode_mcu chosen (the exe's address), its tables built, the bit buffer and EOB run emptied.
void __cdecl jdphuff_start_pass_phuff_decoder(jpeg_decompress_struct *cinfo);
// 0x645030, 0x645300, 0x645560, 0x645660: decode_mcu for a first DC scan (each block's DC difference, shifted by Al), a first AC scan
// (MCU_data[0]'s band Ss..Se, with EOB runs), and their refinements (a bit for each nonzero coefficient, new ones +-1 << Al; an AC
// refinement that suspends takes back the coefficients it made nonzero). FALSE: suspended.
unsigned char __cdecl jdphuff_decode_mcu_DC_first(jpeg_decompress_struct *cinfo, short (**MCU_data)[64]);
unsigned char __cdecl jdphuff_decode_mcu_AC_first(jpeg_decompress_struct *cinfo, short (**MCU_data)[64]);
unsigned char __cdecl jdphuff_decode_mcu_DC_refine(jpeg_decompress_struct *cinfo, short (**MCU_data)[64]);
unsigned char __cdecl jdphuff_decode_mcu_AC_refine(jpeg_decompress_struct *cinfo, short (**MCU_data)[64]);
// 0x645270: process_restart: as jdhuff.c's, the EOB run emptied too.
unsigned char __cdecl jdphuff_process_restart(jpeg_decompress_struct *cinfo);

// jdcoefct.c (through wrap/jpeg-6b/jdcoefct.c): the coefficient buffer controller between the entropy decoder and the IDCT.
// cinfo->coef is its my_coef_controller (pub (start_input_pass, consume_data, start_output_pass, decompress_data, coef_arrays),
// MCU_ctr +0x14, MCU_vert_offset +0x18, MCU_rows_per_iMCU_row +0x1c, MCU_buffer[10] +0x20, whole_image[10] +0x48,
// coef_bits_latch +0x70; 0x74 bytes). The decompress methods take (cinfo, output_buf) and answer JPEG_SUSPENDED (0),
// JPEG_ROW_COMPLETED (3) or JPEG_SCAN_COMPLETED (4).
// 0x643240: the module's init (a shim, jpeg6b.cpp): with a full buffer, each component's virtual block array
// (mem->request_virt_barray) and the multi-scan methods, else one MCU's blocks (mem->alloc_large) and the one-pass ones.
void __cdecl jinit_d_coef_controller(jpeg_decompress_struct *cinfo, unsigned char need_full_buffer);
// 0x643350: start_input_pass: input_iMCU_row zeroed, then start_iMCU_row.
void __cdecl jdcoefct_start_input_pass(jpeg_decompress_struct *cinfo);
// 0x643370: the iMCU row's MCU rows (1 for an interleaved scan, else the component's v_samp_factor or last_row_height), the
// counters zeroed.
void __cdecl jdcoefct_start_iMCU_row(jpeg_decompress_struct *cinfo);
// 0x6433d0: start_output_pass (a shim, jpeg6b.cpp): with the virtual arrays, decompress_data or (do_block_smoothing and
// smoothing_ok) decompress_smooth_data; output_iMCU_row zeroed.
void __cdecl jdcoefct_start_output_pass(jpeg_decompress_struct *cinfo);
// 0x643420: a single-scan iMCU row: each MCU zeroed, decoded (entropy->decode_mcu) and its needed blocks through the IDCT.
int __cdecl jdcoefct_decompress_onepass(jpeg_decompress_struct *cinfo, unsigned char ***output_buf);
// 0x643680: a multi-scan iMCU row decoded into the virtual arrays (mem->access_virt_barray, writable).
int __cdecl jdcoefct_consume_data(jpeg_decompress_struct *cinfo);
// 0x643890: input consumed up to the output row (inputctl->consume_input), then each needed component's blocks of the
// output iMCU row through the IDCT.
int __cdecl jdcoefct_decompress_data(jpeg_decompress_struct *cinfo, unsigned char ***output_buf);
// 0x643a40: progressive with coef_bits, every component's quant_table entries 0, 1, 2, 8, 9 and 16 nonzero, its DC known:
// coef_bits 1-5 latched (alloc_small once), TRUE if any is nonzero.
unsigned char __cdecl jdcoefct_smoothing_ok(jpeg_decompress_struct *cinfo);
// 0x643b70: as decompress_data, with the zero low-frequency AC coefficients estimated from the neighbouring blocks' DC values
// (JPEG K.8) in a copy of each block (jcopy_block_row) before the IDCT.
int __cdecl jdcoefct_decompress_smooth_data(jpeg_decompress_struct *cinfo, unsigned char ***output_buf);

// jdmainct.c (through wrap/jpeg-6b/jdmainct.c): the main buffer controller between the coefficient controller and the
// postprocessor. cinfo->main is its my_main_controller (pub, buffer[10] +8, buffer_full +0x30, rowgroup_ctr +0x34, xbuffer[2] +0x38,
// whichptr +0x40, context_state +0x44, rowgroups_avail +0x48, iMCU_row_ctr +0x4c; 0x50 bytes). M is min_DCT_scaled_size, a
// component's rgroup v_samp_factor * DCT_scaled_size / M rows. The process_data methods take (cinfo, output_buf, out_row_ctr,
// out_rows_avail).
// 0x642b30: the module's init (a shim, jpeg6b.cpp): cinfo->main allocated with the exe's start_pass, the pointer lists when the
// upsampler needs context rows (M + 2 row groups, else M), each component's buffer (mem->alloc_sarray).
void __cdecl jinit_d_main_controller(jpeg_decompress_struct *cinfo, unsigned char need_full_buffer);
// 0x642c10: the two xbuffer lists, each component's 2 * rgroup * (M + 4) row pointers, one row group in.
void __cdecl jdmainct_alloc_funny_pointers(jpeg_decompress_struct *cinfo);
// 0x642cc0: start_pass (a shim, jpeg6b.cpp): the pass's process_data by pass_mode (a J_BUF_MODE; the context lists made), the
// buffer emptied.
void __cdecl jdmainct_start_pass_main(jpeg_decompress_struct *cinfo, int pass_mode);
// 0x642d40: the xbuffer lists from the buffer's rows, the second with the last two row groups swapped, the first with the top
// row group above repeating the first row.
void __cdecl jdmainct_make_funny_pointers(jpeg_decompress_struct *cinfo);
// 0x642ea0: no context: an iMCU row from the coefficient controller into the buffer, then the postprocessor over it.
void __cdecl jdmainct_process_data_simple_main(jpeg_decompress_struct *cinfo, unsigned char **output_buf, unsigned int *out_row_ctr,
                                               unsigned int out_rows_avail);
// 0x642f10: with context rows: the state machine over the two xbuffer lists (a postponed row group, then the iMCU row).
void __cdecl jdmainct_process_data_context_main(jpeg_decompress_struct *cinfo, unsigned char **output_buf,
                                                unsigned int *out_row_ctr, unsigned int out_rows_avail);
// 0x643050: after the first iMCU row: the wraparound row groups above and below each list.
void __cdecl jdmainct_set_wraparound_pointers(jpeg_decompress_struct *cinfo);
// 0x643170: at the last iMCU row: the rows past each component's bottom repeat its last row; rowgroups_avail from component 0.
void __cdecl jdmainct_set_bottom_pointers(jpeg_decompress_struct *cinfo);
// 0x643210: the two-pass quantizer's second pass: the postprocessor alone.
void __cdecl jdmainct_process_data_crank_post(jpeg_decompress_struct *cinfo, unsigned char **output_buf, unsigned int *out_row_ctr,
                                              unsigned int out_rows_avail);

// jdpostct.c (through wrap/jpeg-6b/jdpostct.c): the postprocessing controller between the upsampler and the colour quantizer.
// cinfo->post is its my_post_controller (pub, whole_image +8, buffer +0xc, strip_height +0x10, starting_row +0x14, next_row +0x18;
// 0x1c bytes). The post_process_data methods take (cinfo, input_buf, in_row_group_ctr, in_row_groups_avail, output_buf,
// out_row_ctr, out_rows_avail).
// 0x645d50: the module's init (a shim, jpeg6b.cpp): cinfo->post allocated with the exe's start_pass; when quantizing, a strip of
// max_v_samp_factor rows: a virtual array (need_full_buffer, two-pass) or a strip buffer.
void __cdecl jinit_d_post_controller(jpeg_decompress_struct *cinfo, unsigned char need_full_buffer);
// 0x645de0: start_pass (a shim, jpeg6b.cpp): the pass's post_process_data by pass_mode (a J_BUF_MODE: the upsampler's own method
// when not quantizing; ERREXIT for a mode the buffer cannot serve), the strip emptied.
void __cdecl jdpostct_start_pass_dpost(jpeg_decompress_struct *cinfo, int pass_mode);
// 0x645ee0: one-pass quantizing: a strip upsampled into the buffer, then quantized into output_buf.
void __cdecl jdpostct_post_process_1pass(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int *in_row_group_ctr,
                                         unsigned int in_row_groups_avail, unsigned char **output_buf, unsigned int *out_row_ctr,
                                         unsigned int out_rows_avail);
// 0x645f60: the two-pass prepass: upsampled into the virtual array's strip, which the quantizer scans (no output).
void __cdecl jdpostct_post_process_prepass(jpeg_decompress_struct *cinfo, unsigned char ***input_buf,
                                           unsigned int *in_row_group_ctr, unsigned int in_row_groups_avail,
                                           unsigned char **output_buf, unsigned int *out_row_ctr, unsigned int out_rows_avail);
// 0x646010: the second pass: the virtual array's rows quantized into output_buf.
void __cdecl jdpostct_post_process_2pass(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int *in_row_group_ctr,
                                         unsigned int in_row_groups_avail, unsigned char **output_buf, unsigned int *out_row_ctr,
                                         unsigned int out_rows_avail);

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
// 0x646860: the module's init (a shim, jpeg6b.cpp): cinfo->cconvert allocated (alloc_small) and set up for jpeg_color_space ->
// out_color_space (out_color_components, the converter, build_ycc_rgb_table), with the exe's method addresses.
void __cdecl jinit_color_deconverter(jpeg_decompress_struct *cinfo);
// 0x646d70: YCCK -> CMYK: the YCbCr part through the tables, inverted (range_limit[MAXJSAMPLE - ...]), K passed through.
void __cdecl jdcolor_ycck_cmyk_convert(jpeg_decompress_struct *cinfo, unsigned char ***input_buf, unsigned int input_row,
                                       unsigned char **output_buf, int num_rows);

// jdmerge.c (through wrap/jpeg-6b/jdmerge.c): the merged upsampler, chroma upsampling and YCbCr -> RGB in one pass for h2v1 and h2v2
// sampling. cinfo->upsample is its my_upsampler (upmethod +0xc, the four tables +0x10..+0x1c, spare_row +0x20, spare_full +0x24,
// out_row_width +0x28, rows_to_go +0x2c).
// 0x646ee0: the module's init (a shim, jpeg6b.cpp): cinfo->upsample allocated and set up (the h2v1 or h2v2 methods, spare_row for
// h2v2, the tables), with the exe's method addresses.
void __cdecl jinit_merged_upsampler(jpeg_decompress_struct *cinfo);
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
// 0x6460c0: the module's init (a shim, jpeg6b.cpp): cinfo->upsample allocated, each component's method chosen from its sampling
// factors (h_expand / v_expand for int_upsample), color_buf rows allocated (alloc_sarray) where one is needed, with the exe's method
// addresses.
void __cdecl jinit_upsampler(jpeg_decompress_struct *cinfo);
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

// jdmarker.c (through wrap/jpeg-6b/jdmarker.c): the marker reader, cinfo->marker (my_marker_reader, 0xa8 bytes: pub
// (reset_marker_reader, read_markers, read_restart_marker, saw_SOI +0xc, saw_SOF +0xd, next_restart_num +0x10, discarded_bytes
// +0x14), process_COM +0x18, process_APPn[16] +0x1c, length_limit_COM +0x5c, length_limit_APPn[16] +0x60, cur_marker +0xa0,
// bytes_read +0xa4). Each reads the marker's bytes from cinfo->src (fill_input_buffer when the buffer is used up) and answers FALSE
// (suspended) when a refill does; the trace and error messages go through cinfo->err.
// 0x63bb10: the module's init (a shim, jpeg6b.cpp): cinfo->marker allocated (JPOOL_PERMANENT), the methods (COM and APPn skipped,
// APP0 and APP14 examined), reset, with the exe's method addresses.
void __cdecl jinit_marker_reader(jpeg_decompress_struct *cinfo);
// 0x63d3a0: comp_info, input_scan_number, unread_marker, saw_SOI / SOF, discarded_bytes and cur_marker cleared.
void __cdecl jdmarker_reset_marker_reader(jpeg_decompress_struct *cinfo);
// 0x63c0e0: markers up to SOS (1, JPEG_REACHED_SOS) or EOI (2), or 0 when suspended; the first must be SOI.
int __cdecl jdmarker_read_markers(jpeg_decompress_struct *cinfo);
// 0x63d300: the expected RSTn (next_restart_num), or the source's resync_to_restart; next_restart_num advanced.
unsigned char __cdecl jdmarker_read_restart_marker(jpeg_decompress_struct *cinfo);
// 0x63b8f0: the default resync_to_restart: a marker before or after the desired RSTn read past or left (JWRN_MUST_RESYNC).
unsigned char __cdecl jpeg_resync_to_restart(jpeg_decompress_struct *cinfo, int desired);
// 0x63b9f0: the next marker (unread_marker), any bytes before it counted (JWRN_EXTRANEOUS_DATA).
unsigned char __cdecl jdmarker_next_marker(jpeg_decompress_struct *cinfo);
// 0x63d260: the first two bytes, which must be SOI.
unsigned char __cdecl jdmarker_first_marker(jpeg_decompress_struct *cinfo);
// 0x63c470: SOI: the per-image defaults (arithmetic conditioning, restart_interval, JFIF / Adobe fields), saw_SOI.
unsigned char __cdecl jdmarker_get_soi(jpeg_decompress_struct *cinfo);
// 0x63c530: SOFn: the frame (precision, height, width, components; comp_info allocated once), saw_SOF.
unsigned char __cdecl jdmarker_get_sof(jpeg_decompress_struct *cinfo, unsigned char is_prog, unsigned char is_arith);
// 0x63c850: SOS: the scan's components (cur_comp_info, their table numbers), Ss / Se / Ah / Al, input_scan_number.
unsigned char __cdecl jdmarker_get_sos(jpeg_decompress_struct *cinfo);
// 0x63cb60: DHT: Huffman tables into dc_ / ac_huff_tbl_ptrs (jpeg_alloc_huff_table when null).
unsigned char __cdecl jdmarker_get_dht(jpeg_decompress_struct *cinfo);
// 0x63cee0: DQT: quantization tables (8- or 16-bit, in zigzag order) into quant_tbl_ptrs (jpeg_alloc_quant_table when null).
unsigned char __cdecl jdmarker_get_dqt(jpeg_decompress_struct *cinfo);
// 0x63d170: DRI: restart_interval.
unsigned char __cdecl jdmarker_get_dri(jpeg_decompress_struct *cinfo);
// 0x63c030: a marker's length read and the rest skipped (src->skip_input_data): COM, the other APPn, DAC, DNL.
unsigned char __cdecl jdmarker_skip_variable(jpeg_decompress_struct *cinfo);
// 0x63bb80: APP0 / APP14: up to 14 bytes examined, the rest skipped.
unsigned char __cdecl jdmarker_get_interesting_appn(jpeg_decompress_struct *cinfo);
// 0x63bcc0: an APP0's first datalen bytes: JFIF (saw_JFIF_marker, version, density) or JFXX, else only traced.
void __cdecl jdmarker_examine_app0(jpeg_decompress_struct *cinfo, unsigned char *data, unsigned int datalen, long remaining);
// 0x63bf50: an APP14's: Adobe (saw_Adobe_marker, Adobe_transform), else only traced.
void __cdecl jdmarker_examine_app14(jpeg_decompress_struct *cinfo, unsigned char *data, unsigned int datalen, long remaining);

// jdinput.c (through wrap/jpeg-6b/jdinput.c): the input controller, cinfo->inputctl (my_input_controller, 0x18 bytes: pub
// (consume_input, reset_input_controller, start_input_pass, finish_input_pass, has_multiple_scans +0x10, eoi_reached +0x11),
// inheaders +0x14).
// 0x63b2c0: the module's init (a shim, jpeg6b.cpp): cinfo->inputctl allocated (JPOOL_PERMANENT), consume_markers to consume input,
// with the exe's method addresses.
void __cdecl jinit_input_controller(jpeg_decompress_struct *cinfo);
// 0x63b8b0: back to reading headers (a shim: consume_input is consume_markers again), the error manager and the marker reader reset.
void __cdecl jdinput_reset_input_controller(jpeg_decompress_struct *cinfo);
// 0x63b5e0: consume_input while reading markers: read_markers, then the first SOS's initial_setup or a later one's start_input_pass,
// or EOI.
int __cdecl jdinput_consume_markers(jpeg_decompress_struct *cinfo);
// 0x63b6b0: after the first SOS: the image's checks and dimensions (components' blocks, downsampled sizes, total_iMCU_rows).
void __cdecl jdinput_initial_setup(jpeg_decompress_struct *cinfo);
// 0x63b310: a scan's start: per_scan_setup, latch_quant_tables, the entropy decoder's and coefficient controller's start passes,
// consume_input the coefficient controller's consume_data.
void __cdecl jdinput_start_input_pass(jpeg_decompress_struct *cinfo);
// 0x63b360: the scan's MCU geometry (MCUs_per_row, MCU_rows_in_scan, blocks_in_MCU, MCU_membership, each component's MCU sizes).
void __cdecl jdinput_per_scan_setup(jpeg_decompress_struct *cinfo);
// 0x63b510: each scan component's quantization table copied (JPOOL_IMAGE) the first time it is used.
void __cdecl jdinput_latch_quant_tables(jpeg_decompress_struct *cinfo);
// 0x63b5c0: a scan's end (a shim): consume_input back to consume_markers.
void __cdecl jdinput_finish_input_pass(jpeg_decompress_struct *cinfo);

// jquant1.c (through wrap/jpeg-6b/jquant1.c): the one-pass colour quantizer, cinfo->cquantize (my_cquantizer, 0x58 bytes: pub
// (start_pass, color_quantize, finish_pass, new_color_map), sv_colormap +0x10, sv_actual +0x14, colorindex +0x18, is_padded +0x1c,
// Ncolors[4] +0x20, row_index +0x30, odither[4] +0x34, fserrors[4] +0x44, on_odd_row +0x54). Its finish_pass_1_quant is empty: the
// exe's is the shared bare RET at 0x631270.
// 0x6488f0: the module's init (a shim, jpeg6b.cpp): cinfo->cquantize allocated (JPOOL_IMAGE), the colormap and colorindex made,
// the Floyd-Steinberg workspace for JDITHER_FS, with the exe's method addresses.
void __cdecl jinit_1pass_quantizer(jpeg_decompress_struct *cinfo);
// 0x648db0: a pass's start (a shim): colormap and actual_number_of_colors from the saved ones, color_quantize by dither_mode (and
// out_color_components 3), the ordered-dither tables or the error rows made or cleared.
void __cdecl jquant1_start_pass_1_quant(jpeg_decompress_struct *cinfo, unsigned char is_pre_scan);
// 0x648b00: the colours per component (into Ncolors) for desired_number_of_colors; the total.
int __cdecl jquant1_select_ncolors(jpeg_decompress_struct *cinfo, int *Ncolors);
// 0x648bf0: the j-th of maxj + 1 output values (j * 255 + maxj / 2) / maxj.
int __cdecl jquant1_output_value(jpeg_decompress_struct *cinfo, int ci, int j, int maxj);
// 0x648d50: the largest input value mapped to the j-th output value ((2j + 1) * 255 + maxj) / (2 maxj).
int __cdecl jquant1_largest_input_value(jpeg_decompress_struct *cinfo, int ci, int j, int maxj);
// 0x648990: the colormap (alloc_sarray), sv_colormap and sv_actual.
void __cdecl jquant1_create_colormap(jpeg_decompress_struct *cinfo);
// 0x648c10: colorindex (alloc_sarray; padded by 255 either side for ordered dither, is_padded).
void __cdecl jquant1_create_colorindex(jpeg_decompress_struct *cinfo);
// 0x648f10: a 16 x 16 ordered-dither matrix for ncolors (alloc_small JPOOL_IMAGE).
int *__cdecl jquant1_make_odither_array(jpeg_decompress_struct *cinfo, int ncolors);
// 0x648ea0: odither[] for each component, shared between components with the same Ncolors.
void __cdecl jquant1_create_odither_tables(jpeg_decompress_struct *cinfo);
// 0x648f80 / 0x649040 (three components): rows quantized without dithering.
void __cdecl jquant1_color_quantize(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf, int num_rows);
void __cdecl jquant1_color_quantize3(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf, int num_rows);
// 0x649100 / 0x649230 (three components): with ordered dithering (row_index advanced).
void __cdecl jquant1_quantize_ord_dither(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf,
                                         int num_rows);
void __cdecl jquant1_quantize3_ord_dither(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf,
                                          int num_rows);
// 0x649360: with Floyd-Steinberg dithering (serpentine by on_odd_row, the errors in fserrors).
void __cdecl jquant1_quantize_fs_dither(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf,
                                        int num_rows);
// 0x648d70: fserrors[], (output_width + 2) errors a component (alloc_large JPOOL_IMAGE).
void __cdecl jquant1_alloc_fs_workspace(jpeg_decompress_struct *cinfo);
// 0x649530: ERREXIT JERR_MODE_CHANGE (a one-pass quantizer's colormap cannot change).
void __cdecl jquant1_new_color_map_1_quant(jpeg_decompress_struct *cinfo);

// jquant2.c (through wrap/jpeg-6b/jquant2.c): the two-pass colour quantizer, cinfo->cquantize (my_cquantizer, 0x2c bytes: pub,
// sv_colormap +0x10, desired +0x14, histogram +0x18 (32 rows of 64 x 32 UINT16 cells), needs_zeroed +0x1c, fserrors +0x20, on_odd_row
// +0x24, error_limiter +0x28); boxes of 0x20 bytes (c0min..c2max, volume +0x18, colorcount +0x1c). Its finish_pass2 is empty (the exe's
// is the shared bare RET 0x631270); find_biggest_color_pop (0x6486e0) stays the exe's, the hand port Records32_MaxWeight.
// 0x6474f0: the module's init (a shim, jpeg6b.cpp): cinfo->cquantize, the histogram, sv_colormap when enable_2pass_quant, the F-S
// workspace and error limiter, with the exe's method addresses.
void __cdecl jinit_2pass_quantizer(jpeg_decompress_struct *cinfo);
// 0x6476c0: a pass's start (a shim): the pre-scan's or the second pass's color_quantize and finish_pass, the F-S workspace cleared,
// the histogram zeroed when needs_zeroed.
void __cdecl jquant2_start_pass_2_quant(jpeg_decompress_struct *cinfo, unsigned char is_pre_scan);
// 0x647620: the error limiter's 511 ints (error_limiter 255 in).
void __cdecl jquant2_init_error_limit(jpeg_decompress_struct *cinfo);
// 0x6477e0: pass 1: each pixel's histogram cell counted (saturating).
void __cdecl jquant2_prescan_quantize(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf, int num_rows);
// 0x647860 / 0x647db0: pass 2 without dithering / with Floyd-Steinberg, through the histogram as the inverse colormap's cache.
void __cdecl jquant2_pass2_no_dither(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf, int num_rows);
void __cdecl jquant2_pass2_fs_dither(jpeg_decompress_struct *cinfo, unsigned char **input_buf, unsigned char **output_buf, int num_rows);
// 0x647930: the inverse colormap for the 4 x 8 x 4 cells around (c0, c1, c2).
void __cdecl jquant2_fill_inverse_cmap(jpeg_decompress_struct *cinfo, int c0, int c1, int c2);
// 0x647a50: the colours that can be nearest to some cell of the box at (minc0, minc1, minc2) into colorlist; their count.
int __cdecl jquant2_find_nearby_colors(jpeg_decompress_struct *cinfo, int minc0, int minc1, int minc2, unsigned char *colorlist);
// 0x647c50: each of the box's 128 cells' nearest colour of colorlist into bestcolor.
void __cdecl jquant2_find_best_colors(jpeg_decompress_struct *cinfo, int minc0, int minc1, int minc2, int numcolors,
                                      unsigned char *colorlist, unsigned char *bestcolor);
// 0x6480a0: pass 1's end: the colormap selected (select_colors), needs_zeroed.
void __cdecl jquant2_finish_pass1(jpeg_decompress_struct *cinfo);
// 0x6480d0: median cut over the histogram to desired_colors boxes, one colour each; actual_number_of_colors.
void __cdecl jquant2_select_colors(jpeg_decompress_struct *cinfo, int desired_colors);
// 0x648160: a box shrunk to its non-zero cells; its volume and colour count.
void __cdecl jquant2_update_box(jpeg_decompress_struct *cinfo, void *boxp);
// 0x6485a0: boxes split until desired_colors (or nothing splits); the count.
int __cdecl jquant2_median_cut(jpeg_decompress_struct *cinfo, void *boxlist, int numboxes, int desired_colors);
// 0x648720: the box with the greatest volume (NULL when none is above 0).
void *__cdecl jquant2_find_biggest_volume(void *boxlist, int numboxes);
// 0x648750: a box's mean colour into colormap[][icolor].
void __cdecl jquant2_compute_color(jpeg_decompress_struct *cinfo, void *boxp, int icolor);
// 0x6488e0: needs_zeroed (the histogram is cleared at the next start_pass).
void __cdecl jquant2_new_color_map_2_quant(jpeg_decompress_struct *cinfo);
}
