/* third_party/wrap/jpeg-6b/jquant2.c: jpeg-6b/jquant2.c, unchanged, with its static functions external as jquant2_<name> (see
   wrap/jpeg-6b/jdcolor.c). jinit_2pass_quantizer and start_pass_2_quant store method addresses, so they are built as
   jquant2_lib_<name>: src/game/lib/jpeg6b.cpp's shims call them and put the exe's addresses in their place. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define prescan_quantize jquant2_prescan_quantize
#define find_biggest_color_pop jquant2_find_biggest_color_pop
#define find_biggest_volume jquant2_find_biggest_volume
#define update_box jquant2_update_box
#define median_cut jquant2_median_cut
#define compute_color jquant2_compute_color
#define select_colors jquant2_select_colors
#define find_nearby_colors jquant2_find_nearby_colors
#define find_best_colors jquant2_find_best_colors
#define fill_inverse_cmap jquant2_fill_inverse_cmap
#define pass2_no_dither jquant2_pass2_no_dither
#define pass2_fs_dither jquant2_pass2_fs_dither
#define init_error_limit jquant2_init_error_limit
#define finish_pass1 jquant2_finish_pass1
#define finish_pass2 jquant2_finish_pass2
#define start_pass_2_quant jquant2_lib_start_pass_2_quant
#define new_color_map_2_quant jquant2_new_color_map_2_quant
#define jinit_2pass_quantizer jquant2_lib_jinit_2pass_quantizer

#include "../../jpeg-6b/jquant2.c"  /* (by its path: a plain "jquant2.c" would find this file first) */

/* The exe allocates 0x2c bytes (histogram +0x18, needs_zeroed +0x1c, fserrors +0x20, on_odd_row +0x24, error_limiter +0x28); a box is
   0x20 bytes (find_biggest_color_pop, the hand port Records32_MaxWeight, walks them by 0x20). */
typedef char jquant2_size_0x2c[sizeof(my_cquantizer) == 0x2c ? 1 : -1];
typedef char jquant2_box_0x20[sizeof(box) == 0x20 ? 1 : -1];
