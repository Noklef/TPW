// IJG libjpeg 6b: see jpeg6b.h. The functions are the library's (third_party/jpeg-6b, tpw_jpeg6b); this unit pins the layouts the
// exe's copies share with the library's code while both run (the offsets are the listing's).
#include "jpeg6b.h"

#include <cstddef>
#include <cstdio>

extern "C" {
#include "jpeglib.h"
}

static_assert(sizeof(jpeg_decompress_struct) == 0x1b0, "jpeg_decompress_struct (jpeg_CreateDecompress checks 0x1b0)");
static_assert(sizeof(jpeg_component_info) == 0x54, "jpeg_component_info");
static_assert(sizeof(JCOEF) == 2 && sizeof(JSAMPLE) == 1 && sizeof(JDIMENSION) == 4, "the sample and coefficient types");
// Fields the exe's code addresses directly (listing offsets).
static_assert(offsetof(jpeg_decompress_struct, sample_range_limit) == 0x120, "cinfo->sample_range_limit (the IDCTs' range_limit)");
static_assert(offsetof(jpeg_component_info, dct_table) == 0x50, "compptr->dct_table (the IDCTs' quantptr)");
