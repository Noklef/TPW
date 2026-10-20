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
