// IJG libjpeg 6a: see jpeg6a.h. The functions are the library's (third_party/jpeg-6a, tpw_jpeg6a); this unit checks that its
// declarations are the library's and pins the layouts the exe's copies share with it.
#include "jpeg6a.h"

#include <cstddef>
#include <cstdio>

// 6a's own headers, by path (the include path finds 6b's jpeglib.h and jconfig.h first), under the library's external names. Its
// struct tags are renamed to jpeg6a.h's (6b's jpeg6b.h has the same tags, laid out differently), and jpeglib.h's declarations of the
// functions jpeg6a.h types with tags of its own for the untagged JQUANT_TBL / JHUFF_TBL are renamed away (nothing here calls them).
#include "jpeg6a_prefix.h"
#define jpeg_common_struct jpeg6a_common_struct
#define jpeg_compress_struct jpeg6a_compress_struct
#define jpeg6a_jpeg_alloc_quant_table jpeglib_h_jpeg6a_jpeg_alloc_quant_table
#define jpeg6a_jpeg_alloc_huff_table jpeglib_h_jpeg6a_jpeg_alloc_huff_table
#define jpeg6a_jpeg_set_colorspace jpeglib_h_jpeg6a_jpeg_set_colorspace  // (a J_COLOR_SPACE, an int in jpeg6a.h)
extern "C" {
#define JPEG_INTERNALS
#include "../../../third_party/config/jpeg-6a/jconfig.h"
#define JCONFIG_INCLUDED
#include "../../../third_party/jpeg-6a/jpeglib.h"
#include "../../../third_party/jpeg-6a/jdct.h"
}
#undef jpeg6a_jpeg_alloc_quant_table
#undef jpeg6a_jpeg_alloc_huff_table
#undef jpeg6a_jpeg_set_colorspace

static_assert(JPEG_LIB_VERSION == 61, "libjpeg 6a (jpeg_CreateCompress checks 61)");
static_assert(sizeof(jpeg_compress_struct) == 0x168, "jpeg_compress_struct (jpeg_CreateCompress checks 0x168)");
static_assert(sizeof(DCTELEM) == 4 && sizeof(JCOEF) == 2 && sizeof(JSAMPLE) == 1 && sizeof(JDIMENSION) == 4,
              "the sample, coefficient and DCT types");
static_assert(sizeof(boolean) == 4, "boolean (an int in the exe's 6a: third_party/config/jpeg-6a/jconfig.h)");
// Fields the exe's code addresses directly (listing offsets).
static_assert(offsetof(jpeg_common_struct, mem) == 0x4 && offsetof(jpeg_common_struct, is_decompressor) == 0xc &&
                  offsetof(jpeg_common_struct, global_state) == 0x10,
              "cinfo->mem, is_decompressor, global_state (jpeg_abort 0x64f3c0, jpeg_destroy)");
static_assert(sizeof(JQUANT_TBL) == 0x84 && offsetof(JQUANT_TBL, sent_table) == 0x80, "JQUANT_TBL (jpeg_alloc_quant_table 0x64f43e)");
static_assert(sizeof(JHUFF_TBL) == 0x118 && offsetof(JHUFF_TBL, sent_table) == 0x114, "JHUFF_TBL (jpeg_alloc_huff_table 0x64f46f)");
static_assert(offsetof(jpeg_compress_struct, input_components) == 0x20 && offsetof(jpeg_compress_struct, in_color_space) == 0x24 &&
                  offsetof(jpeg_compress_struct, data_precision) == 0x30 && offsetof(jpeg_compress_struct, num_components) == 0x34 &&
                  offsetof(jpeg_compress_struct, jpeg_color_space) == 0x38 && offsetof(jpeg_compress_struct, comp_info) == 0x3c,
              "the colour fields and comp_info (jpeg_set_colorspace 0x64d23b)");
static_assert(offsetof(jpeg_compress_struct, quant_tbl_ptrs) == 0x40 && offsetof(jpeg_compress_struct, dc_huff_tbl_ptrs) == 0x50 &&
                  offsetof(jpeg_compress_struct, ac_huff_tbl_ptrs) == 0x60 && offsetof(jpeg_compress_struct, arith_dc_L) == 0x70 &&
                  offsetof(jpeg_compress_struct, num_scans) == 0xa0 && offsetof(jpeg_compress_struct, scan_info) == 0xa4,
              "the tables and the scan script (jpeg_set_defaults 0x64cf20)");
static_assert(offsetof(jpeg_compress_struct, raw_data_in) == 0xa8 && offsetof(jpeg_compress_struct, CCIR601_sampling) == 0xb4 &&
                  offsetof(jpeg_compress_struct, write_JFIF_header) == 0xc8 && offsetof(jpeg_compress_struct, density_unit) == 0xcc &&
                  offsetof(jpeg_compress_struct, Y_density) == 0xd0 && offsetof(jpeg_compress_struct, write_Adobe_marker) == 0xd4,
              "the int booleans and the JFIF fields (jpeg_set_defaults)");
static_assert(sizeof(jpeg_component_info) == 0x54 && sizeof(jpeg_scan_info) == 0x24, "a component (10 at 0x348), a scan");
static_assert(offsetof(jpeg_memory_mgr, free_pool) == 0x24 && offsetof(jpeg_memory_mgr, self_destruct) == 0x28,
              "mem->free_pool, self_destruct (jcomapi.c)");
