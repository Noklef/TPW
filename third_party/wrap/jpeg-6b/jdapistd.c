/* third_party/wrap/jpeg-6b/jdapistd.c: jpeg-6b/jdapistd.c, unchanged, with its static output_pass_setup external as
   jdapistd_output_pass_setup (see wrap/jpeg-6b/jdcolor.c). It stores no method addresses. jpeg_read_raw_data, jpeg_start_output and
   jpeg_finish_output are not in the exe (nothing calls them): these copies are built but not hooked. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef LOCAL
#define LOCAL(type) type

#define output_pass_setup jdapistd_output_pass_setup

#include "../../jpeg-6b/jdapistd.c"  /* (by its path: a plain "jdapistd.c" would find this file first) */
