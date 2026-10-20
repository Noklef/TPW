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
extern "C" {
#define JPEG_INTERNALS
#include "../../../third_party/config/jpeg-6a/jconfig.h"
#define JCONFIG_INCLUDED
#include "../../../third_party/jpeg-6a/jpeglib.h"
#include "../../../third_party/jpeg-6a/jdct.h"
}
#undef jpeg6a_jpeg_alloc_quant_table
#undef jpeg6a_jpeg_alloc_huff_table

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
static_assert(offsetof(jpeg_memory_mgr, free_pool) == 0x24 && offsetof(jpeg_memory_mgr, self_destruct) == 0x28,
              "mem->free_pool, self_destruct (jcomapi.c)");
