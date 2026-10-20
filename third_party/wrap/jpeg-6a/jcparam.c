/* third_party/wrap/jpeg-6a/jcparam.c: jpeg-6a/jcparam.c, unchanged, with its static functions external as jpeg6a_jcparam_<name>, so
   the hook table can point the exe's copies of them at these (src/game/lib/jpeg6a.h). As 6b's wrappers (wrap/jpeg-6b/jdcolor.c):
   jmorecfg.h defines METHODDEF and LOCAL as static unconditionally, so they are redefined after it; the names take 6a's jpeg6a_
   prefix as its externals do (config/jpeg6a_prefix.h), so they cannot meet 6b's. tpw_jpeg() compiles this in place of the module. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define add_huff_table jpeg6a_jcparam_add_huff_table
#define std_huff_tables jpeg6a_jcparam_std_huff_tables
#define fill_a_scan jpeg6a_jcparam_fill_a_scan
#define fill_scans jpeg6a_jcparam_fill_scans
#define fill_dc_scans jpeg6a_jcparam_fill_dc_scans

#include "../../jpeg-6a/jcparam.c"  /* (by its path: a plain "jcparam.c" would find this file first) */
