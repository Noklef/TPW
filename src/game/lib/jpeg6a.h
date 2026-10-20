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
#include <cstdint>

extern "C" {

struct jpeg6a_common_struct;    // jpeg_common_struct: err +0, mem +4, progress +8, is_decompressor +0xc, global_state +0x10
struct jpeg6a_compress_struct;  // jpeg_compress_struct, 0x168 bytes (6a's boolean an int)
struct jpeg6a_quant_tbl;        // JQUANT_TBL (untagged), 0x84 bytes (sent_table at +0x80)
struct jpeg6a_huff_tbl;         // JHUFF_TBL (untagged), 0x118 bytes (sent_table at +0x114)

// jcapimin.c / jcapistd.c: the compressor's interface. Error codes are jpeg-6a/jerror.h's.
// 0x64c5f0: mem NULL; version 61 and structsize 0x168 checked (JERR_BAD_LIB_VERSION 0xa, JERR_BAD_STRUCT_SIZE 0x13); the struct zeroed
// but err, then the memory manager (jinit_memory_mgr), the table pointers NULL, input_gamma 1.0, global_state CSTATE_START (100).
void __cdecl jpeg6a_jpeg_CreateCompress(jpeg6a_compress_struct *cinfo, int version, std::size_t structsize);
// 0x64c740, 0x64c751: jpeg_destroy, jpeg_abort.
void __cdecl jpeg6a_jpeg_destroy_compress(jpeg6a_compress_struct *cinfo);
void __cdecl jpeg6a_jpeg_abort_compress(jpeg6a_compress_struct *cinfo);
// 0x64c762: every quantization and Huffman table's sent_table set to suppress.
void __cdecl jpeg6a_jpeg_suppress_tables(jpeg6a_compress_struct *cinfo, int suppress);
// 0x64c7fd: the first pass finished (JERR_TOO_LITTLE_DATA 0x42 below image_height; JERR_BAD_STATE 0x12 outside CSTATE_SCANNING /
// RAW_OK / WRCOEFS), the remaining passes run through the master and the coefficient controller (JERR_CANT_SUSPEND 0x16), then the
// file trailer, term_destination and jpeg_abort.
void __cdecl jpeg6a_jpeg_finish_compress(jpeg6a_compress_struct *cinfo);
// 0x64c982: the marker writer's write_any_marker, before the first scanline only (JERR_BAD_STATE).
void __cdecl jpeg6a_jpeg_write_marker(jpeg6a_compress_struct *cinfo, int marker, const unsigned char *dataptr, unsigned int datalen);
// 0x64cb3c: up to the rows left of num_lines through the main controller's process_data (JERR_BAD_STATE outside CSTATE_SCANNING,
// JWRN_TOO_MUCH_DATA 0x77 past the image), after the progress monitor and the master's pass_startup; the rows taken.
unsigned int __cdecl jpeg6a_jpeg_write_scanlines(jpeg6a_compress_struct *cinfo, unsigned char **scanlines, unsigned int num_lines);
// 0x64cc5e: one iMCU row of raw data through the coefficient controller (JERR_BAD_STATE outside CSTATE_RAW_OK, JERR_BUFFER_SIZE 0x15
// below max_v_samp_factor * 8 lines); its line count, 0 when it suspends or past the image.
unsigned int __cdecl jpeg6a_jpeg_write_raw_data(jpeg6a_compress_struct *cinfo, unsigned char ***data, unsigned int num_lines);
// 0x64c9f4: a tables-only datastream (JERR_BAD_STATE outside CSTATE_START): reset_error_mgr, init_destination, jinit_marker_writer,
// its write_tables_only, term_destination, jpeg_abort.
void __cdecl jpeg6a_jpeg_write_tables(jpeg6a_compress_struct *cinfo);

// jcmarker.c, the marker writer, through third_party/wrap/jpeg-6a/jcmarker.c. Bytes go to cinfo->dest (next_output_byte +0,
// free_in_buffer +4, empty_output_buffer +0xc: JERR_CANT_SUSPEND 0x16 when it answers FALSE).
// 0x64f4a0: cinfo->marker (alloc_small of the image pool, 0x18) and its six methods. Through the method shim (jpeg6a.cpp).
void __cdecl jpeg6a_jinit_marker_writer(jpeg6a_compress_struct *cinfo);
// 0x64f520: marker, the length and datalen bytes of dataptr, when datalen is at most 65533.
void __cdecl jpeg6a_jcmarker_write_any_marker(jpeg6a_compress_struct *cinfo, int marker, const unsigned char *dataptr,
                                              unsigned int datalen);
// 0x64f580, 0x64f5eb, 0x64f611 (static): a byte; 0xff and the marker code; a 16-bit value, high byte first.
void __cdecl jpeg6a_jcmarker_emit_byte(jpeg6a_compress_struct *cinfo, int val);
void __cdecl jpeg6a_jcmarker_emit_marker(jpeg6a_compress_struct *cinfo, int mark);
void __cdecl jpeg6a_jcmarker_emit_2bytes(jpeg6a_compress_struct *cinfo, int value);
// 0x64f644: SOI, then the JFIF APP0 (0x64f68a, static) when write_JFIF_header, the Adobe APP14 (0x64f776, static: its transform 1
// for YCbCr, 2 for YCCK) when write_Adobe_marker.
void __cdecl jpeg6a_jcmarker_write_file_header(jpeg6a_compress_struct *cinfo);
void __cdecl jpeg6a_jcmarker_emit_jfif_app0(jpeg6a_compress_struct *cinfo);
void __cdecl jpeg6a_jcmarker_emit_adobe_app14(jpeg6a_compress_struct *cinfo);
// 0x64f852: each component's DQT, then SOF0 (baseline), SOF1, SOF2 (progressive) or SOF9 (arithmetic); JTRC_16BIT_TABLES (0x4a) for
// a baseline frame with 16-bit tables.
void __cdecl jpeg6a_jcmarker_write_frame_header(jpeg6a_compress_struct *cinfo);
// 0x64f9c5 (static): table index's DQT unless sent (JERR_NO_QUANT_TABLE 0x33 for none), sent_table TRUE; 1 when a value is over 255.
int __cdecl jpeg6a_jcmarker_emit_dqt(jpeg6a_compress_struct *cinfo, int index);
// 0x64fb05 (static): the SOF marker code (JERR_IMAGE_TOO_BIG 0x28 past 65535).
void __cdecl jpeg6a_jcmarker_emit_sof(jpeg6a_compress_struct *cinfo, int code);
// 0x64fc34: the scan's Huffman tables (emit_dac with arithmetic coding, the exe's empty hand port 0x64fe7d), DRI with a restart
// interval, SOS.
void __cdecl jpeg6a_jcmarker_write_scan_header(jpeg6a_compress_struct *cinfo);
// 0x64fd34 (static): a DC or AC (is_ac) table's DHT unless sent (JERR_NO_HUFF_TABLE 0x31 for none), sent_table TRUE.
void __cdecl jpeg6a_jcmarker_emit_dht(jpeg6a_compress_struct *cinfo, int index, int is_ac);
// 0x64fe82, 0x64febc (static): DRI with restart_interval; SOS with the scan's components and tables, Ss, Se, Ah, Al.
void __cdecl jpeg6a_jcmarker_emit_dri(jpeg6a_compress_struct *cinfo);
void __cdecl jpeg6a_jcmarker_emit_sos(jpeg6a_compress_struct *cinfo);
// 0x65000f: EOI. 0x650025: SOI, every quantization table's DQT, the Huffman tables' DHTs (without arithmetic coding), EOI.
void __cdecl jpeg6a_jcmarker_write_file_trailer(jpeg6a_compress_struct *cinfo);
void __cdecl jpeg6a_jcmarker_write_tables_only(jpeg6a_compress_struct *cinfo);

// jcmaster.c, the master control, through third_party/wrap/jpeg-6a/jcmaster.c.
// 0x655ea0: cinfo->master (alloc_small of the image pool, 0x24) and its three methods (through the method shim, jpeg6a.cpp);
// initial_setup, validate_script with a scan script (else one sequential scan), optimize_coding forced when progressive; the first
// pass_type (main, or the transcoder's Huffman-optimisation / output pass), total_passes num_scans (twice with optimize_coding).
void __cdecl jpeg6a_jinit_c_master_control(jpeg6a_compress_struct *cinfo, int transcode_only);
// 0x655fc0 (static): JERR_EMPTY_IMAGE 0x1f, JERR_IMAGE_TOO_BIG 0x28 past 65500, JERR_WIDTH_OVERFLOW 0x45, JERR_BAD_PRECISION 0xd,
// JERR_COMPONENT_COUNT 0x18, JERR_BAD_SAMPLING 0x10; each component's index, sizes in blocks and samples; total_iMCU_rows.
void __cdecl jpeg6a_jcmaster_initial_setup(jpeg6a_compress_struct *cinfo);
// 0x6562fc (static): the scan script checked (JERR_BAD_SCAN_SCRIPT 0x11, JERR_BAD_PROG_SCRIPT 0xf, JERR_MISSING_DATA 0x2c), progressive
// when the first scan's Ss or Se or Ah or Al is not the sequential 0, 63, 0, 0.
void __cdecl jpeg6a_jcmaster_validate_script(jpeg6a_compress_struct *cinfo);
// 0x65691e: the modules started for the pass by pass_type (JERR_NOT_COMPILED 0x2f for another), the scan's headers on an output
// pass, is_last_pass, the progress monitor's counts.
void __cdecl jpeg6a_jcmaster_prepare_for_pass(jpeg6a_compress_struct *cinfo);
// 0x656bb2 (static): the scan's components and Ss..Al, from the script or all of them (JERR_COMPONENT_COUNT past 4).
void __cdecl jpeg6a_jcmaster_select_scan_parameters(jpeg6a_compress_struct *cinfo);
// 0x656d38 (static): MCUs_per_row, MCU_rows_in_scan, blocks_in_MCU and MCU_membership (JERR_BAD_MCU_SIZE 0xb past 10),
// restart_interval from restart_in_rows.
void __cdecl jpeg6a_jcmaster_per_scan_setup(jpeg6a_compress_struct *cinfo);
// 0x65701a: the frame header (the first scan only) and the scan header; 0x657055: the entropy coder's finish_pass, then the next
// pass_type, scan_number and pass_number.
void __cdecl jpeg6a_jcmaster_pass_startup(jpeg6a_compress_struct *cinfo);
void __cdecl jpeg6a_jcmaster_finish_pass_master(jpeg6a_compress_struct *cinfo);

// jcmainct.c, the main buffer controller, through third_party/wrap/jpeg-6a/jcmainct.c (without FULL_MAIN_BUFFER_SUPPORTED, as the
// exe's build: a full-image buffer is JERR_BAD_BUFFER_MODE 4).
// 0x6504c0: cinfo->main (alloc_small of the image pool, 0x40) and start_pass (through the method shim, jpeg6a.cpp); unless
// raw_data_in, a strip of v_samp_factor * 8 rows for each component (alloc_sarray).
void __cdecl jpeg6a_jinit_c_main_controller(jpeg6a_compress_struct *cinfo, int need_full_buffer);
// 0x650589: the counters cleared and pass_mode kept, process_data_simple_main the process_data for JBUF_PASS_THRU (through its own
// shim), JERR_BAD_BUFFER_MODE for another mode; nothing in raw-data mode.
void __cdecl jpeg6a_jcmainct_start_pass_main(jpeg6a_compress_struct *cinfo, int pass_mode);
// 0x650608: up to total_iMCU_rows iMCU rows, each filled by the preprocessor's pre_process_data and handed to the coefficient
// controller's compress_data; when that suspends, in_row_ctr one back until the row is taken.
void __cdecl jpeg6a_jcmainct_process_data_simple_main(jpeg6a_compress_struct *cinfo, unsigned char **input_buf,
                                                      unsigned int *in_row_ctr, unsigned int in_rows_avail);

// jcprepct.c, the preprocessing controller, through third_party/wrap/jpeg-6a/jcprepct.c (with CONTEXT_ROWS_SUPPORTED, as the exe's).
// 0x654090: cinfo->prep (alloc_small of the image pool, 0x40) and its methods (through the method shim, jpeg6a.cpp); a full buffer
// is JERR_BAD_BUFFER_MODE 4; when the downsampler needs context rows, pre_process_context and create_context_buffer, else
// pre_process_data and a row group of max_v_samp_factor rows for each component (alloc_sarray).
void __cdecl jpeg6a_jinit_c_prep_controller(jpeg6a_compress_struct *cinfo, int need_full_buffer);
// 0x654188 (static): JBUF_PASS_THRU only (JERR_BAD_BUFFER_MODE); rows_to_go image_height, the buffer empty, next_buf_stop two row
// groups.
void __cdecl jpeg6a_jcprepct_start_pass_prep(jpeg6a_compress_struct *cinfo, int pass_mode);
// 0x6541ed (static): rows colour-converted into color_buf until a row group is there, then downsampled into output_buf; at the
// image's bottom the last row repeated, in color_buf and then in output_buf to its full height.
void __cdecl jpeg6a_jcprepct_pre_process_data(jpeg6a_compress_struct *cinfo, unsigned char **input_buf, unsigned int *in_row_ctr,
                                              unsigned int in_rows_avail, unsigned char ***output_buf,
                                              unsigned int *out_row_group_ctr, unsigned int out_row_groups_avail);
// 0x6543fc (static): rows input_rows..output_rows-1 copies of row input_rows-1 (jcopy_sample_rows).
void __cdecl jpeg6a_jcprepct_expand_bottom_edge(unsigned char **image_data, unsigned int num_cols, int input_rows, int output_rows);
// 0x654440 (static): pre_process_data with a row group of context above and below, in color_buf's wraparound of three row groups
// (the top row copied above the image first, the last below it at the bottom).
void __cdecl jpeg6a_jcprepct_pre_process_context(jpeg6a_compress_struct *cinfo, unsigned char **input_buf,
                                                 unsigned int *in_row_ctr, unsigned int in_rows_avail,
                                                 unsigned char ***output_buf, unsigned int *out_row_group_ctr,
                                                 unsigned int out_row_groups_avail);
// 0x6546a5 (static): each component's three row groups (alloc_sarray) behind five groups of row pointers (alloc_small), the first
// and last the wraparound's.
void __cdecl jpeg6a_jcprepct_create_context_buffer(jpeg6a_compress_struct *cinfo);

// jccoefct.c, the coefficient buffer controller, through third_party/wrap/jpeg-6a/jccoefct.c (with FULL_COEF_BUFFER_SUPPORTED, as
// the exe's). Blocks are 64 shorts (JBLOCK).
// 0x650700: cinfo->coef (alloc_small of the image pool, 0x68) and start_pass (through the method shim, jpeg6a.cpp); a full buffer is
// a virtual block array for each component, padded to whole MCUs (request_virt_barray), else ten blocks (alloc_large) behind
// MCU_buffer and whole_image[0] null.
void __cdecl jpeg6a_jinit_c_coef_controller(jpeg6a_compress_struct *cinfo, int need_full_buffer);
// 0x650818 (static, through its own shim): iMCU_row_num 0, the row's counters; compress_data for JBUF_PASS_THRU (no virtual arrays),
// compress_first_pass for JBUF_SAVE_AND_PASS, compress_output for JBUF_CRANK_DEST (with them), else JERR_BAD_BUFFER_MODE 4.
void __cdecl jpeg6a_jccoefct_start_pass_coef(jpeg6a_compress_struct *cinfo, int pass_mode);
// 0x650908 (static): MCU_rows_per_iMCU_row (1 for an interleaved scan, else the component's v_samp_factor, or last_row_height in
// the last iMCU row), mcu_ctr and MCU_vert_offset 0.
void __cdecl jpeg6a_jccoefct_start_iMCU_row(jpeg6a_compress_struct *cinfo);
// 0x650982 (static): an iMCU row's MCUs, each block row through the forward DCT into MCU_buffer (dummy blocks past the right and
// bottom edges: zeroed, the previous block's DC), each MCU to the entropy encoder; FALSE when it suspends, the counters kept.
int __cdecl jpeg6a_jccoefct_compress_data(jpeg6a_compress_struct *cinfo, unsigned char ***input_buf);
// 0x650c30 (static): every component's iMCU row through the forward DCT into its virtual array (dummy blocks at the right edge and,
// in the last iMCU row, dummy block rows, the DC of each MCU's last real block), then compress_output.
int __cdecl jpeg6a_jccoefct_compress_first_pass(jpeg6a_compress_struct *cinfo, unsigned char ***input_buf);
// 0x650ee3 (static): the scan's MCUs of the iMCU row from the virtual arrays to the entropy encoder (MCU_buffer pointing into them).
int __cdecl jpeg6a_jccoefct_compress_output(jpeg6a_compress_struct *cinfo, unsigned char ***input_buf);

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
// 0x64cf20: the defaults (JERR_BAD_STATE unless CSTATE_START): comp_info (alloc_small of 10 entries when null), 8-bit precision,
// jpeg_set_quality(75, TRUE), the standard Huffman tables, the arithmetic conditioning, no scan script, no raw data, arithmetic or
// optimised coding, CCIR601 sampling, smoothing, restarts; JDCT_ISLOW, density 1:1, then jpeg_default_colorspace.
void __cdecl jpeg6a_jpeg_set_defaults(jpeg6a_compress_struct *cinfo);
// 0x64d0a8 (static): the four standard Huffman tables (DC / AC, luminance / chrominance) through add_huff_table. Each huffval is
// copied whole (256 bytes) from a shorter list, so past its symbols it holds the library's neighbouring bytes, not the exe's: no code
// reads past the symbols (jpeg6a_param.py's mask has the proof).
void __cdecl jpeg6a_jcparam_std_huff_tables(jpeg6a_compress_struct *cinfo);
// 0x64d121 (static): *htblptr (jpeg_alloc_huff_table when null) given bits[17] and huffval[256] copied from bits and val, sent_table
// FALSE.
void __cdecl jpeg6a_jcparam_add_huff_table(jpeg6a_compress_struct *cinfo, jpeg6a_huff_tbl **htblptr, const unsigned char *bits,
                                           const unsigned char *val);
// 0x64d188: jpeg_set_colorspace by in_color_space (RGB to YCbCr, the others to themselves); JERR_BAD_IN_COLORSPACE (7) for another.
void __cdecl jpeg6a_jpeg_default_colorspace(jpeg6a_compress_struct *cinfo);
// 0x64d23b: jpeg_color_space, the JFIF / Adobe marker flags, num_components and each component's id, sampling and table numbers for
// colorspace (J_COLOR_SPACE; JCS_UNKNOWN takes input_components, JERR_COMPONENT_COUNT (0x18) outside 1..10, the components set all
// the same); JERR_BAD_STATE, JERR_BAD_J_COLORSPACE (8).
void __cdecl jpeg6a_jpeg_set_colorspace(jpeg6a_compress_struct *cinfo, int colorspace);
// 0x64d859: a progressive scan script in a permanent block (10 scans for 3-component YCbCr, else 2 + 4 a component, 6 a component
// past four), through fill_dc_scans / fill_a_scan / fill_scans; JERR_BAD_STATE.
void __cdecl jpeg6a_jpeg_simple_progression(jpeg6a_compress_struct *cinfo);
struct jpeg6a_scan_info;  // jpeg_scan_info (untagged), 0x24 bytes
// 0x64dab2, 0x64daf9, 0x64db5f (static): one scan of component ci; a scan for each of ncomps components; the DC scan of ncomps
// components together (one scan a component past four). Each returns the scan after the last it wrote.
jpeg6a_scan_info *__cdecl jpeg6a_jcparam_fill_a_scan(jpeg6a_scan_info *scanptr, int ci, int Ss, int Se, int Ah, int Al);
jpeg6a_scan_info *__cdecl jpeg6a_jcparam_fill_scans(jpeg6a_scan_info *scanptr, int ncomps, int Ss, int Se, int Ah, int Al);
jpeg6a_scan_info *__cdecl jpeg6a_jcparam_fill_dc_scans(jpeg6a_scan_info *scanptr, int ncomps, int Ah, int Al);

// jmemansi.c, the memory manager's system-dependent part, through third_party/wrap/jpeg-6a/jmemansi.c: its malloc and free are the
// exe's C runtime's (Jpeg6a_SetCrtEnv below), so every block stays on the exe's heap.
// 0x650210, 0x650232: malloc(sizeofobject).
void *__cdecl jpeg6a_jpeg_get_small(jpeg6a_common_struct *cinfo, std::size_t sizeofobject);
void *__cdecl jpeg6a_jpeg_get_large(jpeg6a_common_struct *cinfo, std::size_t sizeofobject);
// 0x650221, 0x650243: free(object).
void __cdecl jpeg6a_jpeg_free_small(jpeg6a_common_struct *cinfo, void *object, std::size_t sizeofobject);
void __cdecl jpeg6a_jpeg_free_large(jpeg6a_common_struct *cinfo, void *object, std::size_t sizeofobject);
// 0x650254: mem->max_memory_to_use (+0x2c) - already_allocated.
long __cdecl jpeg6a_jpeg_mem_available(jpeg6a_common_struct *cinfo, long min_bytes_needed, long max_bytes_needed, long already_allocated);

// jmemmgr.c, the memory manager, through third_party/wrap/jpeg-6a/jmemmgr.c: cinfo->mem (my_memory_mgr, 0x50 bytes: the public
// jpeg_memory_mgr 0x30 (no max_alloc_chunk in 6a), small_list[2] +0x30, large_list[2] +0x38, virt_sarray_list +0x40,
// virt_barray_list +0x44, total_space_allocated +0x48, last_rowsperchunk +0x4c); pools 0 (JPOOL_PERMANENT) and 1 (JPOOL_IMAGE); a
// pool's blocks start with a 16-byte header (next, bytes_used, bytes_left). Its getenv / sscanf and jpeg_open_backing_store are the
// exe's (Jpeg6aCrtEnv below).
struct jpeg6a_jvirt_sarray_control;  // 0x80 bytes (int booleans pre_zero +0x20, dirty +0x24, b_s_open +0x28; the backing store +0x30)
struct jpeg6a_jvirt_barray_control;  // 0x80 bytes
typedef short jpeg6a_block[64];      // JBLOCK
// 0x64e020: the manager (jpeg_get_small of 0x50, JERR_OUT_OF_MEMORY 0x35 without it), its eleven methods, max_memory_to_use from
// jpeg_mem_init, or JPEGMEM's (thousands, or millions with an m / M); cinfo->mem. Through the method shim (jpeg6a.cpp).
void __cdecl jpeg6a_jinit_memory_mgr(jpeg6a_common_struct *cinfo);
// 0x64e3e3 (static): ERREXIT1 JERR_OUT_OF_MEMORY (0x35) with which.
void __cdecl jpeg6a_jmemmgr_out_of_memory(jpeg6a_common_struct *cinfo, int which);
// 0x64e21d: sizeofobject (rounded up to 8) from the first small pool of pool_id with room, else a new pool (jpeg_get_small of the
// object, its header and the pool's slop, the slop halved while it fails; out_of_memory below 50).
void *__cdecl jpeg6a_jmemmgr_alloc_small(jpeg6a_common_struct *cinfo, int pool_id, std::size_t sizeofobject);
// 0x64e40d: a large object of its own (jpeg_get_large), linked into large_list[pool_id].
void *__cdecl jpeg6a_jmemmgr_alloc_large(jpeg6a_common_struct *cinfo, int pool_id, std::size_t sizeofobject);
// 0x64e4ff, 0x64e600: numrows rows (pointers from alloc_small, the rows from alloc_large in chunks of last_rowsperchunk).
unsigned char **__cdecl jpeg6a_jmemmgr_alloc_sarray(jpeg6a_common_struct *cinfo, int pool_id, unsigned int samplesperrow,
                                                    unsigned int numrows);
jpeg6a_block **__cdecl jpeg6a_jmemmgr_alloc_barray(jpeg6a_common_struct *cinfo, int pool_id, unsigned int blocksperrow,
                                                   unsigned int numrows);
// 0x64e715, 0x64e7ba: a virtual array's control block (JPOOL_IMAGE only), unrealized, at the head of its list.
jpeg6a_jvirt_sarray_control *__cdecl jpeg6a_jmemmgr_request_virt_sarray(jpeg6a_common_struct *cinfo, int pool_id, int pre_zero,
                                                                        unsigned int samplesperrow, unsigned int numrows,
                                                                        unsigned int maxaccess);
jpeg6a_jvirt_barray_control *__cdecl jpeg6a_jmemmgr_request_virt_barray(jpeg6a_common_struct *cinfo, int pool_id, int pre_zero,
                                                                        unsigned int blocksperrow, unsigned int numrows,
                                                                        unsigned int maxaccess);
// 0x64e85f: every unrealized array its buffer, all of it in memory when jpeg_mem_available allows, else a backing store.
void __cdecl jpeg6a_jmemmgr_realize_virt_arrays(jpeg6a_common_struct *cinfo);
// 0x64eb3a, 0x64ee6f: rows start_row.. of a virtual array, read in (and the old ones written out) through the backing store when
// they are not in memory, zeroed when undefined and pre_zero.
unsigned char **__cdecl jpeg6a_jmemmgr_access_virt_sarray(jpeg6a_common_struct *cinfo, jpeg6a_jvirt_sarray_control *ptr,
                                                          unsigned int start_row, unsigned int num_rows, int writable);
jpeg6a_block **__cdecl jpeg6a_jmemmgr_access_virt_barray(jpeg6a_common_struct *cinfo, jpeg6a_jvirt_barray_control *ptr,
                                                         unsigned int start_row, unsigned int num_rows, int writable);
// 0x64ed29, 0x64f061 (static): the buffer's chunks read from or written to the backing store.
void __cdecl jpeg6a_jmemmgr_do_sarray_io(jpeg6a_common_struct *cinfo, jpeg6a_jvirt_sarray_control *ptr, int writing);
void __cdecl jpeg6a_jmemmgr_do_barray_io(jpeg6a_common_struct *cinfo, jpeg6a_jvirt_barray_control *ptr, int writing);
// 0x64f1aa: a pool's blocks freed (the image pool's backing stores closed first), large then small.
void __cdecl jpeg6a_jmemmgr_free_pool(jpeg6a_common_struct *cinfo, int pool_id);
// 0x64f360: both pools freed, then the manager; cinfo->mem NULL.
void __cdecl jpeg6a_jmemmgr_self_destruct(jpeg6a_common_struct *cinfo);

// jerror.c's methods, through third_party/wrap/jpeg-6a/jerror.c: cinfo->err (jpeg_error_mgr, 0x84 bytes, 6b's layout: the five
// methods +0, msg_code +0x14, msg_parm +0x18 (eight ints or an 80-character string), trace_level +0x68, num_warnings +0x6c,
// jpeg_message_table +0x70, last_jpeg_message +0x74, addon_message_table +0x78, first_addon_message +0x7c, last_addon_message +0x80).
// error_exit (0x64ddec), output_message (0x64de13) and jpeg_std_error (the hand port Fn_64dd60) stay the exe's.
// 0x64de4f: a warning (msg_level < 0) is output when it is the first or trace_level >= 3, and counted; a trace when trace_level >=
// msg_level.
void __cdecl jpeg6a_jerror_emit_message(jpeg6a_common_struct *cinfo, int msg_level);
// 0x64dead: msg_code's text (the library's table, else the add-on table, else message 0 with msg_code as its parameter) formatted into
// buffer with sprintf (the exe's, Jpeg6aCrtEnv): msg_parm.s when the text's first % is a %s, else the eight ints.
void __cdecl jpeg6a_jerror_format_message(jpeg6a_common_struct *cinfo, char *buffer);
// 0x64dff8: num_warnings and msg_code 0.
void __cdecl jpeg6a_jerror_reset_error_mgr(jpeg6a_common_struct *cinfo);

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

// What the library calls outside itself (Jpeg6aCrtEnv below), defined in jpeg6a.cpp for the wrapper units' #defines.
void *__cdecl jpeg6a_exe_malloc(std::size_t size);
void __cdecl jpeg6a_exe_free(void *block);
char *__cdecl jpeg6a_exe_getenv(const char *name);
int __cdecl jpeg6a_exe_sscanf(const char *s, const char *format, long *value, char *ch);
void __cdecl jpeg6a_exe_open_backing_store(jpeg6a_common_struct *cinfo, void *info, long total_bytes_needed);
int __cdecl jpeg6a_exe_sprintf(char *buffer, const char *format, ...);

}  // extern "C"

// What the library calls outside itself, the exe's, as 6b's (jpeg6b.h's Jpeg6bCrtEnv): its C runtime's malloc / free (jmemansi.c),
// getenv / sscanf of JPEGMEM and the backing store (jmemmgr.c: jpeg_open_backing_store stays the exe's 6a one, 0x650265), and
// format_message's two sprintf calls (jerror.c), told apart by the message text as format_message does. The tests give it stand-ins.
struct Jpeg6aCrtEnv {
    void *(__cdecl *alloc)(std::size_t size);  // malloc (0x67dcd0)
    void(__cdecl *free)(void *block);          // free (0x67ad90)
    char *(__cdecl *getenv)(const char *name);                                       // getenv (0x67fbf0)
    int(__cdecl *sscanf)(const char *s, const char *format, long *value, char *ch);  // sscanf (0x67f620): jmemmgr.c's "%ld%c"
    void(__cdecl *open_backing_store)(jpeg6a_common_struct *cinfo, void *info, long total_bytes_needed);  // the exe's (0x650265)
    int(__cdecl *sprintf_string)(char *buffer, const char *format, const char *s);  // sprintf (0x67a650), format_message's %s call
    int(__cdecl *sprintf_ints)(char *buffer, const char *format, int i0, int i1, int i2, int i3, int i4, int i5, int i6,
                               int i7);  // sprintf (0x67a650), its other call
};

// The library's calls go through env (nullptr: the exe's).
void Jpeg6a_SetCrtEnv(const Jpeg6aCrtEnv *env);

// For the tests' driver, as 6b's (jpeg6b.h's Jpeg6b_SetNativeMethods): with native methods the shims leave the library's method
// addresses in place, so a case can run a method an init it called stored, and Jpeg6a_ExeMethod gives the exe's address of a library
// method's (0 for anything else), which the driver puts back in the memory it reports.
void Jpeg6a_SetNativeMethods(bool native);
std::uint32_t Jpeg6a_ExeMethod(std::uint32_t address);
