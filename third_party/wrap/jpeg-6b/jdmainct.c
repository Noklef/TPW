/* third_party/wrap/jpeg-6b/jdmainct.c: jpeg-6b/jdmainct.c, unchanged, with its static functions external as jdmainct_<name> (see
   wrap/jpeg-6b/jdcolor.c). jinit_d_main_controller and start_pass_main store method addresses (start_pass, and the pass's
   process_data), so they are built as jdmainct_lib_<name>: src/game/lib/jpeg6b.cpp's shims (jinit_d_main_controller,
   jdmainct_start_pass_main) call them and put the exe's addresses in their place. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define alloc_funny_pointers jdmainct_alloc_funny_pointers
#define make_funny_pointers jdmainct_make_funny_pointers
#define set_wraparound_pointers jdmainct_set_wraparound_pointers
#define set_bottom_pointers jdmainct_set_bottom_pointers
#define process_data_simple_main jdmainct_process_data_simple_main
#define process_data_context_main jdmainct_process_data_context_main
#define process_data_crank_post jdmainct_process_data_crank_post
#define start_pass_main jdmainct_lib_start_pass_main
#define jinit_d_main_controller jdmainct_lib_jinit_d_main_controller

#include "../../jpeg-6b/jdmainct.c"  /* (by its path: a plain "jdmainct.c" would find this file first) */
