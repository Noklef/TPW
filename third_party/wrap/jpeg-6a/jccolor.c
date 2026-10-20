/* third_party/wrap/jpeg-6a/jccolor.c: jpeg-6a/jccolor.c, unchanged, with its static functions external as jpeg6a_jccolor_<name>
   (see wrap/jpeg-6a/jcparam.c). jinit_color_converter stores start_pass and color_convert, so it is built as
   jpeg6a_jccolor_lib_jinit_color_converter: src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's addresses in their place
   (null_method's is the exe's empty function 0x655e8d, a hand port). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type

#define rgb_ycc_start jpeg6a_jccolor_rgb_ycc_start
#define rgb_ycc_convert jpeg6a_jccolor_rgb_ycc_convert
#define rgb_gray_convert jpeg6a_jccolor_rgb_gray_convert
#define cmyk_ycck_convert jpeg6a_jccolor_cmyk_ycck_convert
#define grayscale_convert jpeg6a_jccolor_grayscale_convert
#define null_convert jpeg6a_jccolor_null_convert
#define null_method jpeg6a_jccolor_null_method
#undef jinit_color_converter  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_color_converter: the shim's name) */
#define jinit_color_converter jpeg6a_jccolor_lib_jinit_color_converter

#include "../../jpeg-6a/jccolor.c"  /* (by its path: a plain "jccolor.c" would find this file first) */

/* The exe allocates 0xc bytes (my_color_converter: the public 8, rgb_ycc_tab). */
typedef char jpeg6a_jccolor_size_0xc[sizeof(my_color_converter) == 0xc ? 1 : -1];
