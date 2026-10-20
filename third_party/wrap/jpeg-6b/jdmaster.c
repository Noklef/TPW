/* third_party/wrap/jpeg-6b/jdmaster.c: jpeg-6b/jdmaster.c, unchanged, with its static functions external as jdmaster_<name> (see
   wrap/jpeg-6b/jdcolor.c). Its methods (prepare_for_output_pass, finish_output_pass) are also the names of jpegint.h's
   jpeg_decomp_master members, so they are defined before the headers: the members are renamed with them, consistently, in this unit
   alone (the layout is the same). jinit_master_decompress stores the two method addresses, so it is built as
   jdmaster_lib_jinit_master_decompress: src/game/lib/jpeg6b.cpp's shim calls it and puts the exe's addresses in their place.
   jpeg_new_colormap is not in the exe (nothing calls it): this copy is built but not hooked. */
#define prepare_for_output_pass jdmaster_prepare_for_output_pass
#define finish_output_pass jdmaster_finish_output_pass

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define use_merged_upsample jdmaster_use_merged_upsample
#define prepare_range_limit_table jdmaster_prepare_range_limit_table
#define master_selection jdmaster_master_selection
#define jinit_master_decompress jdmaster_lib_jinit_master_decompress

#include "../../jpeg-6b/jdmaster.c"  /* (by its path: a plain "jdmaster.c" would find this file first) */

/* The exe allocates 0x1c bytes (quantizer_2pass at +0x18). */
typedef char jdmaster_size_0x1c[sizeof(my_decomp_master) == 0x1c ? 1 : -1];
