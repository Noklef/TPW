// zlib 1.1.3: see zlib.h. The functions are the library's (third_party/zlib-1.1.3, tpw_zlib); this unit pins the layouts the exe's
// deflate copies use, which the library's code and the exe's must share while both run, and holds what every shim uses: the exe's
// copies of the strings the library stores in strm->msg. The inflate side is zlib_inflate.cpp (deflate.h and infutil.h both define
// struct internal_state, so they cannot share a unit).
#include "zlib.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "../core/crt.h"

extern "C" {
#include "zlib_shims.h"  // (the shimmed functions as the library builds them: zlib_<name>)
#include "deflate.h"
#include "inftrees.h"
}
#undef _tr_init
#undef deflateReset
#undef deflateInit2_
#undef deflate

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
    (std::uintptr_t)&gText_Zlib_779adc, (std::uintptr_t)&gText_Zlib_779ac8, (std::uintptr_t)&gText_Zlib_779ab0, (std::uintptr_t)&gText_Zlib_779a98, (std::uintptr_t)&gText_Zlib_779a88,                       // inflate
    (std::uintptr_t)&gText_Zlib_779c4c, (std::uintptr_t)&gText_Zlib_779c2c, (std::uintptr_t)&gText_Zlib_779c08, (std::uintptr_t)&gText_Zlib_779bec,                                 // inflate_blocks
    (std::uintptr_t)&gText_Zlib_77a078, (std::uintptr_t)&gText_Zlib_77a060,                                                     // inflate_codes, inflate_fast
    (std::uintptr_t)&gText_Zlib_77b1c4, (std::uintptr_t)&gText_Zlib_77b1a0, (std::uintptr_t)&gText_Zlib_77b270, (std::uintptr_t)&gText_Zlib_77b254, (std::uintptr_t)&gText_Zlib_77b230, (std::uintptr_t)&gText_Zlib_77b20c, (std::uintptr_t)&gText_Zlib_77b1ec,  // inflate_trees_bits / dynamic
    (std::uintptr_t)&gText_Zlib_779b9c, (std::uintptr_t)&gText_Zlib_779b90, (std::uintptr_t)&gText_Zlib_779b80, (std::uintptr_t)&gText_Zlib_779b74, (std::uintptr_t)&gText_Zlib_779b60, (std::uintptr_t)&gText_Zlib_779b50, (std::uintptr_t)&gText_Zlib_779b38,  // z_errmsg[1], [3..8]
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

namespace {
// The exe's static_l_desc, static_d_desc and static_bl_desc, for the library's (trees.c) that _tr_init stores in the state: the
// state holds the exe's addresses as the exe's _tr_init leaves them.
const std::uintptr_t kExeStaticLDesc = (std::uintptr_t)&gZlibStaticLDesc_779ba8, kExeStaticDDesc = (std::uintptr_t)&gZlibStaticDDesc_779bc0, kExeStaticBlDesc = (std::uintptr_t)&gZlibStaticBlDesc_779bd8;

void exe_descs(deflate_state *s) {
    s->l_desc.stat_desc = reinterpret_cast<static_tree_desc *>(kExeStaticLDesc);
    s->d_desc.stat_desc = reinterpret_cast<static_tree_desc *>(kExeStaticDDesc);
    s->bl_desc.stat_desc = reinterpret_cast<static_tree_desc *>(kExeStaticBlDesc);
}
}  // namespace

extern "C" {

void __cdecl _tr_init(internal_state *s) {
    zlib__tr_init(s);
    exe_descs(s);
}

int __cdecl deflateReset(z_stream_s *strm) {
    const int r = zlib_deflateReset(strm);
    if (r == Z_OK) exe_descs(strm->state);
    return r;
}

int __cdecl deflateInit2_(z_stream_s *strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version,
                          int stream_size) {
    const char *const before = strm ? strm->msg : nullptr;
    const int r = zlib_deflateInit2_(strm, level, method, windowBits, memLevel, strategy, version, stream_size);
    if (r == Z_OK) exe_descs(strm->state);
    zlibshim::exe_message(strm, before);
    return r;
}

int __cdecl deflate(z_stream_s *strm, int flush) {
    const char *const before = strm ? strm->msg : nullptr;
    const int r = zlib_deflate(strm, flush);
    zlibshim::exe_message(strm, before);
    return r;
}

void *__cdecl Zlib_Calloc(void *, unsigned items, unsigned size) { return CrtInner().calloc(items, size); }
}
