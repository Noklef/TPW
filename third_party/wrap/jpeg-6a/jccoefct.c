/* third_party/wrap/jpeg-6a/jccoefct.c: jpeg-6a/jccoefct.c, unchanged, with its static functions external as jpeg6a_jccoefct_<name>
   (see wrap/jpeg-6a/jcparam.c). compress_data is also the name of jpeglib.h's jpeg_c_coef_controller member, so it is defined
   before the headers: the member is renamed with it, consistently, in this unit alone (the layout is the same).
   jinit_c_coef_controller stores start_pass_coef, and start_pass_coef one of the three compress methods, so both are built under
   _lib_ names: src/game/lib/jpeg6a.cpp's shims call them and put the exe's addresses in place of the library's. With 6a's
   ENTROPY_OPT_SUPPORTED (jmorecfg.h), so FULL_COEF_BUFFER_SUPPORTED, as the exe's build (compress_first_pass and compress_output
   are in it). */
#define compress_data jpeg6a_jccoefct_compress_data

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_iMCU_row jpeg6a_jccoefct_start_iMCU_row
#define start_pass_coef jpeg6a_jccoefct_lib_start_pass_coef
#define compress_first_pass jpeg6a_jccoefct_compress_first_pass
#define compress_output jpeg6a_jccoefct_compress_output
#undef jinit_c_coef_controller  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_c_coef_controller: the shim's name) */
#define jinit_c_coef_controller jpeg6a_jccoefct_lib_jinit_c_coef_controller

#include "../../jpeg-6a/jccoefct.c"  /* (by its path: a plain "jccoefct.c" would find this file first) */

/* The exe allocates 0x68 bytes (my_coef_controller: the public 8, four counters, MCU_buffer[10] +0x18, whole_image[10] +0x40). */
typedef char jpeg6a_jccoefct_size_0x68[sizeof(my_coef_controller) == 0x68 ? 1 : -1];
