// zlib 1.1.3: see zlib.h. The functions are the library's (third_party/zlib-1.1.3, tpw_zlib); this unit pins the layouts the exe's
// deflate copies use, which the library's code and the exe's must share while both run, and holds what every shim uses: the exe's
// copies of the strings the library stores in strm->msg. The inflate side is zlib_inflate.cpp (deflate.h and infutil.h both define
// struct internal_state, so they cannot share a unit).
#include "zlib.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

extern "C" {
#include "zlib_shims.h"  // (the shimmed functions as the library builds them: zlib_<name>)
#include "deflate.h"
#include "inftrees.h"
}

// The exe's sizes: inflateInit2_ / deflateInit2_ check the stream size, deflateInit2_ allocates the state,
// inflate_blocks_new the hufts (MANY of them).
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
static_assert(offsetof(z_stream, msg) == 0x18 && offsetof(z_stream, zfree) == 0x24, "z_stream.msg (the exe's code stores [z + 0x18])");

namespace {
// The exe's copies of the strings the library stores in strm->msg. The exe's linker pooled equal literals, so each text is there once
// (inflate_codes and inflate_fast share theirs, inflate's "need dictionary" is z_errmsg's): inflate.c's, infblock.c's, infcodes.c's /
// inffast.c's and inftrees.c's literals, then the entries of the exe's z_errmsg (0x779b10) that deflate.c stores through ERR_MSG.
const std::uintptr_t kExeMessages[] = {
    0x779adc, 0x779ac8, 0x779ab0, 0x779a98, 0x779a88,                       // inflate
    0x779c4c, 0x779c2c, 0x779c08, 0x779bec,                                 // inflate_blocks
    0x77a078, 0x77a060,                                                     // inflate_codes, inflate_fast
    0x77b1c4, 0x77b1a0, 0x77b270, 0x77b254, 0x77b230, 0x77b20c, 0x77b1ec,  // inflate_trees_bits / dynamic
    0x779b9c, 0x779b90, 0x779b80, 0x779b74, 0x779b60, 0x779b50, 0x779b38,  // z_errmsg[1], [3..8]
};
}  // namespace

namespace zlibshim {

void exe_message(z_stream_s *z, const char *before) {
    if (!z || !z->msg || z->msg == before) return;
    for (const std::uintptr_t a : kExeMessages)
        if (!std::strcmp(z->msg, reinterpret_cast<const char *>(a))) {
            z->msg = reinterpret_cast<char *>(a);
            return;
        }
}
}  // namespace zlibshim
