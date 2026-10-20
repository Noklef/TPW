// zlib 1.1.3: see zlib.h. The functions are the library's (third_party/zlib-1.1.3, tpw_zlib); this unit only
// pins the layouts the exe's copies use, which the library's code and the exe's must share while both run.
#include "zlib.h"

#include <cstddef>

extern "C" {
#include "deflate.h"
#include "infblock.h"
#include "inftrees.h"
}

// The exe's sizes: inflateInit2_ / deflateInit2_ check the stream size, deflateInit2_ allocates the state,
// inflate_blocks_new the blocks state and its hufts (MANY of them).
static_assert(sizeof(z_stream) == 0x38, "z_stream (deflateInit2_ / inflateInit2_ check 0x38)");
static_assert(sizeof(deflate_state) == 0x16b8, "deflate_state (deflateInit2_ allocates 0x16b8)");
static_assert(sizeof(inflate_huft) == 8 && MANY == 1440, "inflate_huft (inflate_blocks_new allocates 1440 x 8)");
// Fields the exe's code addresses directly (listing offsets).
static_assert(offsetof(deflate_state, data_type) == 0x1c, "deflate_state.data_type");
static_assert(offsetof(deflate_state, dyn_ltree) == 0x8c, "deflate_state.dyn_ltree");
static_assert(offsetof(deflate_state, l_desc) == 0xb10, "deflate_state.l_desc");
static_assert(offsetof(deflate_state, bl_count) == 0xb34, "deflate_state.bl_count");
static_assert(offsetof(deflate_state, heap) == 0xb54, "deflate_state.heap");
static_assert(offsetof(deflate_state, depth) == 0x1450, "deflate_state.depth");
static_assert(offsetof(deflate_state, l_buf) == 0x1690, "deflate_state.l_buf");
static_assert(offsetof(deflate_state, last_eob_len) == 0x16ac, "deflate_state.last_eob_len");
static_assert(offsetof(deflate_state, bi_buf) == 0x16b0 && offsetof(deflate_state, bi_valid) == 0x16b4, "deflate_state.bi_buf");
