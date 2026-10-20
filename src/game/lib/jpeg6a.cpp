// IJG libjpeg 6a: see jpeg6a.h. The functions are the library's (third_party/jpeg-6a, tpw_jpeg6a); this unit checks that its
// declarations are the library's and pins the layouts the exe's copies share with it.
#include "jpeg6a.h"

#include <cstddef>
#include <cstdio>

// 6a's own headers, by path (the include path finds 6b's jpeglib.h and jconfig.h first), under the library's external names
#include "jpeg6a_prefix.h"
extern "C" {
#define JPEG_INTERNALS
#include "../../../third_party/config/jpeg-6a/jconfig.h"
#define JCONFIG_INCLUDED
#include "../../../third_party/jpeg-6a/jpeglib.h"
#include "../../../third_party/jpeg-6a/jdct.h"
}

static_assert(JPEG_LIB_VERSION == 61, "libjpeg 6a (jpeg_CreateCompress checks 61)");
static_assert(sizeof(jpeg_compress_struct) == 0x168, "jpeg_compress_struct (jpeg_CreateCompress checks 0x168)");
static_assert(sizeof(DCTELEM) == 4 && sizeof(JCOEF) == 2 && sizeof(JSAMPLE) == 1 && sizeof(JDIMENSION) == 4,
              "the sample, coefficient and DCT types");
static_assert(sizeof(boolean) == 4, "boolean (an int in the exe's 6a: third_party/config/jpeg-6a/jconfig.h)");
