/* third_party/wrap/jpeg-6b/jdmerge.c: jpeg-6b/jdmerge.c, unchanged, with its static functions external as jdmerge_<name> (see
   wrap/jpeg-6b/jdcolor.c). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define build_ycc_rgb_table jdmerge_build_ycc_rgb_table
#define start_pass_merged_upsample jdmerge_start_pass_merged_upsample
#define merged_2v_upsample jdmerge_merged_2v_upsample
#define merged_1v_upsample jdmerge_merged_1v_upsample
#define h2v1_merged_upsample jdmerge_h2v1_merged_upsample
#define h2v2_merged_upsample jdmerge_h2v2_merged_upsample

#include "../../jpeg-6b/jdmerge.c"  /* (by its path: a plain "jdmerge.c" would find this file first) */
