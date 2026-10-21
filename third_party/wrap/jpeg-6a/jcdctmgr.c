/* third_party/wrap/jpeg-6a/jcdctmgr.c: jpeg-6a/jcdctmgr.c, unchanged, with its static functions external as jpeg6a_jcdctmgr_<name>
   (see wrap/jpeg-6a/jcparam.c). forward_DCT is also the name of jpeglib.h's jpeg_forward_dct member, so it is defined before the
   headers: the member is renamed with it, consistently, in this unit alone (the layout is the same). jinit_forward_dct stores
   start_pass, forward_DCT and the DCT routine, so it is built as jpeg6a_jcdctmgr_lib_jinit_forward_dct: src/game/lib/jpeg6a.cpp's
   shim calls it and puts the exe's addresses in place of the library's. Compiled /arch:IA32 (third_party/CMakeLists.txt):
   start_pass_fdctmgr's JDCT_FLOAT divisors are x87 code in the exe, run at the FPU's precision. */
#define forward_DCT jpeg6a_jcdctmgr_forward_DCT

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type

#define start_pass_fdctmgr jpeg6a_jcdctmgr_start_pass_fdctmgr
#define forward_DCT_float jpeg6a_jcdctmgr_forward_DCT_float
#undef jinit_forward_dct  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_forward_dct: the shim's name) */
#define jinit_forward_dct jpeg6a_jcdctmgr_lib_jinit_forward_dct

#include "../../jpeg-6a/jcdctmgr.c"  /* (by its path: a plain "jcdctmgr.c" would find this file first) */

/* The exe allocates 0x30 bytes (my_fdct_controller: the public 8, do_dct, divisors[4], do_float_dct, float_divisors[4]). */
typedef char jpeg6a_jcdctmgr_size_0x30[sizeof(my_fdct_controller) == 0x30 ? 1 : -1];
