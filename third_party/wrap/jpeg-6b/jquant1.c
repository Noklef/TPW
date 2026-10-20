/* third_party/wrap/jpeg-6b/jquant1.c: jpeg-6b/jquant1.c, unchanged, with its static functions external as jquant1_<name> (see
   wrap/jpeg-6b/jdcolor.c). color_quantize is also the name of a struct member (jpeglib.h's jpeg_color_quantizer), so it is defined
   before the headers: the member is renamed with it, consistently, in this unit alone. jinit_1pass_quantizer and start_pass_1_quant
   store method addresses, so they are built as jquant1_lib_<name>: src/game/lib/jpeg6b.cpp's shims call them and put the exe's
   addresses in their place. */
#define color_quantize jquant1_color_quantize

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define select_ncolors jquant1_select_ncolors
#define output_value jquant1_output_value
#define largest_input_value jquant1_largest_input_value
#define create_colormap jquant1_create_colormap
#define create_colorindex jquant1_create_colorindex
#define make_odither_array jquant1_make_odither_array
#define create_odither_tables jquant1_create_odither_tables
#define color_quantize3 jquant1_color_quantize3
#define quantize_ord_dither jquant1_quantize_ord_dither
#define quantize3_ord_dither jquant1_quantize3_ord_dither
#define quantize_fs_dither jquant1_quantize_fs_dither
#define alloc_fs_workspace jquant1_alloc_fs_workspace
#define start_pass_1_quant jquant1_lib_start_pass_1_quant
#define finish_pass_1_quant jquant1_finish_pass_1_quant
#define new_color_map_1_quant jquant1_new_color_map_1_quant
#define jinit_1pass_quantizer jquant1_lib_jinit_1pass_quantizer

#include "../../jpeg-6b/jquant1.c"  /* (by its path: a plain "jquant1.c" would find this file first) */

/* The exe allocates 0x58 bytes (row_index at +0x30, odither[] +0x34, fserrors[] +0x44, on_odd_row +0x54). */
typedef char jquant1_size_0x58[sizeof(my_cquantizer) == 0x58 ? 1 : -1];
