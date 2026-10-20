/* third_party/wrap/jpeg-6b/jdpostct.c: jpeg-6b/jdpostct.c, unchanged, with its static functions external as jdpostct_<name> (see
   wrap/jpeg-6b/jdcolor.c). jinit_d_post_controller and start_pass_dpost store method addresses (start_pass, and the pass's
   post_process_data), so they are built as jdpostct_lib_<name>: src/game/lib/jpeg6b.cpp's shims (jinit_d_post_controller,
   jdpostct_start_pass_dpost) call them and put the exe's addresses in their place. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define post_process_1pass jdpostct_post_process_1pass
#define post_process_prepass jdpostct_post_process_prepass
#define post_process_2pass jdpostct_post_process_2pass
#define start_pass_dpost jdpostct_lib_start_pass_dpost
#define jinit_d_post_controller jdpostct_lib_jinit_d_post_controller

#include "../../jpeg-6b/jdpostct.c"  /* (by its path: a plain "jdpostct.c" would find this file first) */
