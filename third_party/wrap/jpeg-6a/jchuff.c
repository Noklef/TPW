/* third_party/wrap/jpeg-6a/jchuff.c: jpeg-6a/jchuff.c, unchanged, with its static functions external as jpeg6a_jchuff_<name>
   (see wrap/jpeg-6a/jcparam.c). jinit_huff_encoder stores start_pass_huff, and start_pass_huff the encode_mcu and finish_pass
   methods for the pass, so both are built under _lib_ names: src/game/lib/jpeg6a.cpp's shims call them and put the exe's addresses
   in place of the library's. With 6a's ENTROPY_OPT_SUPPORTED (jmorecfg.h), as the exe's build (the gather methods are in it). */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_pass_huff jpeg6a_jchuff_lib_start_pass_huff
#define dump_buffer jpeg6a_jchuff_dump_buffer
#define emit_bits jpeg6a_jchuff_emit_bits
#define flush_bits jpeg6a_jchuff_flush_bits
#define encode_one_block jpeg6a_jchuff_encode_one_block
#define emit_restart jpeg6a_jchuff_emit_restart
#define encode_mcu_huff jpeg6a_jchuff_encode_mcu_huff
#define finish_pass_huff jpeg6a_jchuff_finish_pass_huff
#define htest_one_block jpeg6a_jchuff_htest_one_block
#define encode_mcu_gather jpeg6a_jchuff_encode_mcu_gather
#define finish_pass_gather jpeg6a_jchuff_finish_pass_gather
#undef jinit_huff_encoder  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_huff_encoder: the shim's name) */
#define jinit_huff_encoder jpeg6a_jchuff_lib_jinit_huff_encoder

#include "../../jpeg-6a/jchuff.c"  /* (by its path: a plain "jchuff.c" would find this file first) */

/* The exe allocates 0x6c bytes (huff_entropy_encoder: the public 0xc, saved +0xc, restarts_to_go +0x24, next_restart_num +0x28,
   dc / ac_derived_tbls +0x2c / +0x3c, dc / ac_count_ptrs +0x4c / +0x5c), and its working_state is 0x24 bytes (cinfo +0x20). */
typedef char jpeg6a_jchuff_size_0x6c[sizeof(huff_entropy_encoder) == 0x6c ? 1 : -1];
typedef char jpeg6a_jchuff_state_0x24[sizeof(working_state) == 0x24 ? 1 : -1];
