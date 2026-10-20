/* third_party/wrap/jpeg-6b/jdphuff.c: jpeg-6b/jdphuff.c, unchanged, with its static functions external as jdphuff_<name> (see
   wrap/jpeg-6b/jdcolor.c). jinit_phuff_decoder and start_pass_phuff_decoder store method addresses (start_pass, and the scan's
   decode_mcu), so they are built as jdphuff_lib_<name>: src/game/lib/jpeg6b.cpp's shims (jinit_phuff_decoder,
   jdphuff_start_pass_phuff_decoder) call them and put the exe's addresses in their place. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define process_restart jdphuff_process_restart
#define decode_mcu_DC_first jdphuff_decode_mcu_DC_first
#define decode_mcu_AC_first jdphuff_decode_mcu_AC_first
#define decode_mcu_DC_refine jdphuff_decode_mcu_DC_refine
#define decode_mcu_AC_refine jdphuff_decode_mcu_AC_refine
#define start_pass_phuff_decoder jdphuff_lib_start_pass_phuff_decoder
#define jinit_phuff_decoder jdphuff_lib_jinit_phuff_decoder

#include "../../jpeg-6b/jdphuff.c"  /* (by its path: a plain "jdphuff.c" would find this file first) */
