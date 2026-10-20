/* third_party/wrap/jpeg-6b/jdcolor.c: jpeg-6b/jdcolor.c, unchanged, with its static functions external as jdcolor_<name>, so the hook
   table can point the exe's copies of them at these (src/game/lib/jpeg6b.h; docs/porting.md, "Library functions"). jmorecfg.h defines
   METHODDEF and LOCAL as static unconditionally, so they are redefined after it, and each name is renamed first (the modules reuse
   names: jdmerge.c has a build_ycc_rgb_table too). tpw_jpeg() compiles this in place of the module. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define build_ycc_rgb_table jdcolor_build_ycc_rgb_table
#define ycc_rgb_convert jdcolor_ycc_rgb_convert
#define null_convert jdcolor_null_convert
#define grayscale_convert jdcolor_grayscale_convert
#define gray_rgb_convert jdcolor_gray_rgb_convert
#define ycck_cmyk_convert jdcolor_ycck_cmyk_convert
#define start_pass_dcolor jdcolor_start_pass_dcolor
/* (a method-storing init: built as jdcolor_lib_<name>; src/game/lib/jpeg6b.cpp's shim of the library's name calls it and puts the exe's
   method addresses in place of the library's) */
#define jinit_color_deconverter jdcolor_lib_jinit_color_deconverter

#include "../../jpeg-6b/jdcolor.c"  /* (by its path: a plain "jdcolor.c" would find this file first) */
