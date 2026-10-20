/* third_party/wrap/jpeg-6a/jcmainct.c: jpeg-6a/jcmainct.c, unchanged, with its static functions external as jpeg6a_jcmainct_<name>
   (see wrap/jpeg-6a/jcparam.c). jinit_c_main_controller stores start_pass_main, so it is built as
   jpeg6a_jcmainct_lib_jinit_c_main_controller: src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's address in its place
   (start_pass_main's process_data the same, through its own shim). Without FULL_MAIN_BUFFER_SUPPORTED, as the exe's build:
   process_data_buffer_main is not compiled (nor in the exe). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type

#define start_pass_main jpeg6a_jcmainct_lib_start_pass_main
#define process_data_simple_main jpeg6a_jcmainct_process_data_simple_main
#undef jinit_c_main_controller  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_c_main_controller: the shim's name) */
#define jinit_c_main_controller jpeg6a_jcmainct_lib_jinit_c_main_controller

#include "../../jpeg-6a/jcmainct.c"  /* (by its path: a plain "jcmainct.c" would find this file first) */

/* The exe allocates 0x40 bytes (my_main_controller: the public 8, four counters and flags, buffer[10]). */
typedef char jpeg6a_jcmainct_size_0x40[sizeof(my_main_controller) == 0x40 ? 1 : -1];
