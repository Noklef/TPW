/* third_party/wrap/jpeg-6b/jdsample.c: jpeg-6b/jdsample.c, unchanged, with its static functions external as jdsample_<name> (see
   wrap/jpeg-6b/jdcolor.c). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_pass_upsample jdsample_start_pass_upsample
#define sep_upsample jdsample_sep_upsample
#define fullsize_upsample jdsample_fullsize_upsample
#define noop_upsample jdsample_noop_upsample
#define int_upsample jdsample_int_upsample
#define h2v1_upsample jdsample_h2v1_upsample
#define h2v2_upsample jdsample_h2v2_upsample
#define h2v1_fancy_upsample jdsample_h2v1_fancy_upsample
#define h2v2_fancy_upsample jdsample_h2v2_fancy_upsample
/* (a method-storing init: see wrap/jpeg-6b/jdcolor.c) */
#define jinit_upsampler jdsample_lib_jinit_upsampler

#include "../../jpeg-6b/jdsample.c"  /* (by its path: a plain "jdsample.c" would find this file first) */

/* The private methods src/game/lib/jpeg6b.cpp's shim maps by their offset: methods[MAX_COMPONENTS] at +0x34. */
typedef char jdsample_methods_at_0x34[offsetof(my_upsampler, methods) == 0x34 && MAX_COMPONENTS == 10 ? 1 : -1];
