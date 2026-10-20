/* third_party/wrap/jpeg-6b/jdcoefct.c: jpeg-6b/jdcoefct.c, unchanged, with its static functions external as jdcoefct_<name> (see
   wrap/jpeg-6b/jdcolor.c). start_input_pass, consume_data, start_output_pass and decompress_data are also the names of struct
   members (jpeglib.h's jpeg_d_coef_controller, jpeg_input_controller), so they are defined before the headers: the members are renamed
   with them, consistently, in this unit alone. jinit_d_coef_controller and start_output_pass store method addresses, so they are built
   as jdcoefct_lib_<name>: src/game/lib/jpeg6b.cpp's shims (jinit_d_coef_controller, jdcoefct_start_output_pass) call them and put
   the exe's addresses in their place. */
#define start_input_pass jdcoefct_start_input_pass
#define consume_data jdcoefct_consume_data
#define start_output_pass jdcoefct_lib_start_output_pass
#define decompress_data jdcoefct_decompress_data

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_iMCU_row jdcoefct_start_iMCU_row
#define decompress_onepass jdcoefct_decompress_onepass
#define dummy_consume_data jdcoefct_dummy_consume_data
#define smoothing_ok jdcoefct_smoothing_ok
#define decompress_smooth_data jdcoefct_decompress_smooth_data
#define jinit_d_coef_controller jdcoefct_lib_jinit_d_coef_controller

#include "../../jpeg-6b/jdcoefct.c"  /* (by its path: a plain "jdcoefct.c" would find this file first) */
