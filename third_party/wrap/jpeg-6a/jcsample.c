/* third_party/wrap/jpeg-6a/jcsample.c: jpeg-6a/jcsample.c, unchanged, with its static functions external as jpeg6a_jcsample_<name>
   (see wrap/jpeg-6a/jcparam.c). jinit_downsampler stores start_pass, downsample and a method for each component (the private
   methods[]), so it is built as jpeg6a_jcsample_lib_jinit_downsampler: src/game/lib/jpeg6a.cpp's shim calls it and puts the exe's
   addresses in place of the library's (start_pass_downsample's is the exe's empty function 0x654a1d, a hand port). With 6a's
   INPUT_SMOOTHING_SUPPORTED (jmorecfg.h), as the exe's build (h2v2_smooth_downsample and fullsize_smooth_downsample are in it). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_pass_downsample jpeg6a_jcsample_start_pass_downsample
#define expand_right_edge jpeg6a_jcsample_expand_right_edge
#define sep_downsample jpeg6a_jcsample_sep_downsample
#define int_downsample jpeg6a_jcsample_int_downsample
#define fullsize_downsample jpeg6a_jcsample_fullsize_downsample
#define h2v1_downsample jpeg6a_jcsample_h2v1_downsample
#define h2v2_downsample jpeg6a_jcsample_h2v2_downsample
#define h2v2_smooth_downsample jpeg6a_jcsample_h2v2_smooth_downsample
#define fullsize_smooth_downsample jpeg6a_jcsample_fullsize_smooth_downsample
#undef jinit_downsampler  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_downsampler: the shim's name) */
#define jinit_downsampler jpeg6a_jcsample_lib_jinit_downsampler

#include "../../jpeg-6a/jcsample.c"  /* (by its path: a plain "jcsample.c" would find this file first) */

/* The exe allocates 0x34 bytes (my_downsampler: the public 0xc, methods[10] +0xc: jpeg6a.cpp's shim maps them by that offset). */
typedef char jpeg6a_jcsample_size_0x34[sizeof(my_downsampler) == 0x34 ? 1 : -1];
