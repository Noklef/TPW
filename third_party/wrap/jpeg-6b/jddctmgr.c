/* third_party/wrap/jpeg-6b/jddctmgr.c: jpeg-6b/jddctmgr.c, unchanged, with its static start_pass external as jddctmgr_lib_start_pass and
   jinit_inverse_dct built as jddctmgr_lib_jinit_inverse_dct (see wrap/jpeg-6b/jdcolor.c): both store method addresses, so
   src/game/lib/jpeg6b.cpp's shims (jddctmgr_start_pass, jinit_inverse_dct) call these and put the exe's addresses in their place.
   start_pass is also a member name (struct jpeg_inverse_dct's, and the other modules' public structs'), so it is renamed before
   jpeglib.h: the members are renamed with it, consistently in this unit, and the layouts do not change. */
#define start_pass jddctmgr_lib_start_pass

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define jinit_inverse_dct jddctmgr_lib_jinit_inverse_dct

#include "../../jpeg-6b/jddctmgr.c"  /* (by its path: a plain "jddctmgr.c" would find this file first) */
