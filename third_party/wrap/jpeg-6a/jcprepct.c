/* third_party/wrap/jpeg-6a/jcprepct.c: jpeg-6a/jcprepct.c, unchanged, with its static functions external as jpeg6a_jcprepct_<name>
   (see wrap/jpeg-6a/jcparam.c). pre_process_data is also the name of jpeglib.h's jpeg_c_prep_controller member, so it is defined
   before the headers: the member is renamed with it, consistently, in this unit alone (the layout is the same).
   jinit_c_prep_controller stores the three method addresses, so it is built as jpeg6a_jcprepct_lib_jinit_c_prep_controller:
   src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's addresses in their place. With 6a's INPUT_SMOOTHING_SUPPORTED
   (jmorecfg.h), so CONTEXT_ROWS_SUPPORTED, as the exe's build (pre_process_context and create_context_buffer are in it). */
#define pre_process_data jpeg6a_jcprepct_pre_process_data

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_pass_prep jpeg6a_jcprepct_start_pass_prep
#define expand_bottom_edge jpeg6a_jcprepct_expand_bottom_edge
#define pre_process_context jpeg6a_jcprepct_pre_process_context
#define create_context_buffer jpeg6a_jcprepct_create_context_buffer
#undef jinit_c_prep_controller  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_c_prep_controller: the shim's name) */
#define jinit_c_prep_controller jpeg6a_jcprepct_lib_jinit_c_prep_controller

#include "../../jpeg-6a/jcprepct.c"  /* (by its path: a plain "jcprepct.c" would find this file first) */

/* The exe allocates 0x40 bytes (my_prep_controller: the public 8, color_buf[10], rows_to_go, next_buf_row, this_row_group,
   next_buf_stop). */
typedef char jpeg6a_jcprepct_size_0x40[sizeof(my_prep_controller) == 0x40 ? 1 : -1];
