/* third_party/wrap/jpeg-6b/jdhuff.c: jpeg-6b/jdhuff.c, unchanged, with its static functions external as jdhuff_<name> (see
   wrap/jpeg-6b/jdcolor.c) and jinit_huff_decoder built as jdhuff_lib_jinit_huff_decoder: it stores method addresses, so
   src/game/lib/jpeg6b.cpp's shim of that name calls it and puts the exe's addresses in their place. decode_mcu is also a member name
   (struct jpeg_entropy_decoder's), so it is renamed before jpeglib.h: the member is renamed with it, consistently in this unit, and
   the layout does not change. */
#define decode_mcu jdhuff_decode_mcu

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_pass_huff_decoder jdhuff_start_pass_huff_decoder
#define process_restart jdhuff_process_restart
#define jinit_huff_decoder jdhuff_lib_jinit_huff_decoder

#include "../../jpeg-6b/jdhuff.c"  /* (by its path: a plain "jdhuff.c" would find this file first) */
