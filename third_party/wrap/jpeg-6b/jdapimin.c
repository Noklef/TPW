/* third_party/wrap/jpeg-6b/jdapimin.c: jpeg-6b/jdapimin.c, unchanged, with its static default_decompress_parms external as
   jdapimin_default_decompress_parms (see wrap/jpeg-6b/jdcolor.c). It stores no method addresses. jpeg_abort_decompress,
   jpeg_input_complete and jpeg_has_multiple_scans are not in the exe (nothing calls them): these copies are built but not hooked. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef LOCAL
#define LOCAL(type) type

#define default_decompress_parms jdapimin_default_decompress_parms

#include "../../jpeg-6b/jdapimin.c"  /* (by its path: a plain "jdapimin.c" would find this file first) */
