/* third_party/wrap/jpeg-6b/jdinput.c: jpeg-6b/jdinput.c, unchanged, with its static functions external as jdinput_<name> (see
   wrap/jpeg-6b/jdcolor.c). start_input_pass, finish_input_pass and reset_input_controller are also the names of struct members
   (jpeglib.h's jpeg_input_controller; start_input_pass jpeg_d_coef_controller's too), so they are defined before the headers: the
   members are renamed with them, consistently, in this unit alone. jinit_input_controller, finish_input_pass and reset_input_controller
   store method addresses (consume_markers), so they are built as jdinput_lib_<name>: src/game/lib/jpeg6b.cpp's shims call them and put
   the exe's addresses in their place. */
#define start_input_pass jdinput_start_input_pass
#define finish_input_pass jdinput_lib_finish_input_pass
#define reset_input_controller jdinput_lib_reset_input_controller

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define initial_setup jdinput_initial_setup
#define per_scan_setup jdinput_per_scan_setup
#define latch_quant_tables jdinput_latch_quant_tables
#define consume_markers jdinput_consume_markers
#define jinit_input_controller jdinput_lib_jinit_input_controller

#include "../../jpeg-6b/jdinput.c"  /* (by its path: a plain "jdinput.c" would find this file first) */

/* The exe allocates 0x18 bytes (inheaders at +0x14). */
typedef char jdinput_size_0x18[sizeof(my_input_controller) == 0x18 ? 1 : -1];
