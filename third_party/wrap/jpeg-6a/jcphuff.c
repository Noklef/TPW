/* third_party/wrap/jpeg-6a/jcphuff.c: jpeg-6a/jcphuff.c, unchanged, with its static functions external as jpeg6a_jcphuff_<name>
   (see wrap/jpeg-6a/jcparam.c). jinit_phuff_encoder stores start_pass_phuff, and start_pass_phuff the encode_mcu and finish_pass
   methods for the scan, so both are built under _lib_ names: src/game/lib/jpeg6a.cpp's shims call them and put the exe's addresses
   in place of the library's. With 6a's C_PROGRESSIVE_SUPPORTED (jmorecfg.h), as the exe's build. */
#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"

#undef METHODDEF
#define METHODDEF(type) type
#undef LOCAL
#define LOCAL(type) type

#define start_pass_phuff jpeg6a_jcphuff_lib_start_pass_phuff
#define dump_buffer jpeg6a_jcphuff_dump_buffer
#define emit_bits jpeg6a_jcphuff_emit_bits
#define flush_bits jpeg6a_jcphuff_flush_bits
#define emit_symbol jpeg6a_jcphuff_emit_symbol
#define emit_buffered_bits jpeg6a_jcphuff_emit_buffered_bits
#define emit_eobrun jpeg6a_jcphuff_emit_eobrun
#define emit_restart jpeg6a_jcphuff_emit_restart
#define encode_mcu_DC_first jpeg6a_jcphuff_encode_mcu_DC_first
#define encode_mcu_AC_first jpeg6a_jcphuff_encode_mcu_AC_first
#define encode_mcu_DC_refine jpeg6a_jcphuff_encode_mcu_DC_refine
#define encode_mcu_AC_refine jpeg6a_jcphuff_encode_mcu_AC_refine
#define finish_pass_phuff jpeg6a_jcphuff_finish_pass_phuff
#define finish_pass_gather_phuff jpeg6a_jcphuff_finish_pass_gather_phuff
#undef jinit_phuff_encoder  /* (config/jpeg6a_prefix.h's jpeg6a_jinit_phuff_encoder: the shim's name) */
#define jinit_phuff_encoder jpeg6a_jcphuff_lib_jinit_phuff_encoder

#include "../../jpeg-6a/jcphuff.c"  /* (by its path: a plain "jcphuff.c" would find this file first) */

/* The exe allocates 0x6c bytes (phuff_entropy_encoder: the public 0xc, gather_statistics +0xc, next_output_byte +0x10,
   free_in_buffer +0x14, put_buffer +0x18, put_bits +0x1c, cinfo +0x20, last_dc_val[4] +0x24, ac_tbl_no +0x34, EOBRUN +0x38, BE
   +0x3c, bit_buffer +0x40, restarts_to_go +0x44, next_restart_num +0x48, derived_tbls[4] +0x4c, count_ptrs[4] +0x5c). */
typedef char jpeg6a_jcphuff_size_0x6c[sizeof(phuff_entropy_encoder) == 0x6c ? 1 : -1];
