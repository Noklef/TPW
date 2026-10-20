// zlib 1.1.3's inflate side: see zlib.h. The inflate layouts the exe's copies share with the library, and the inflate shims: the
// library's function (built as zlib_<name>, third_party/config/zlib_shims.h), then the exe's copy of a string or table put back where
// the library left its own address.
//
// The fixed tables: the library's inflate_trees_fixed gives its own fixed_tl / fixed_td (inffixed.h), the exe's (0x64a450, kept: see
// docs/porting.md) gives the exe's copies, byte for byte the same tables. A codes state for a fixed block keeps pointers into them
// (ltree, dtree, and the table it is part way through), so the shims of the functions that build one (inflate_blocks, and inflate,
// which runs it) point those at the exe's copies: the live state after the call, and a state the call frees before its zfree sees it
// (z->zfree is wrapped for the call).
#include "zlib.h"

#include <cstddef>
#include <cstdint>

extern "C" {
#include "zlib_shims.h"
#include "zutil.h"
#include "infblock.h"
#include "inftrees.h"
#include "infcodes.h"
#include "infutil.h"
}
#undef inflateInit2_
#undef inflate
#undef inflate_blocks
#undef inflate_codes
#undef inflate_fast
#undef inflate_trees_bits
#undef inflate_trees_dynamic

static_assert(sizeof(inflate_blocks_statef) == 0x40, "inflate_blocks_state (inflate_blocks_new allocates 0x40)");
static_assert(offsetof(inflate_blocks_statef, bitb) == 0x20 && offsetof(inflate_blocks_statef, write) == 0x34, "inflate_blocks_state (inflate_fast)");

extern "C" {
int zlib_inflate_trees_bits(uIntf *c, uIntf *bb, inflate_huft **tb, inflate_huft *hp, z_streamp z);
int zlib_inflate_codes(inflate_blocks_statef *s, z_streamp z, int r);
int zlib_inflate_fast(uInt bl, uInt bd, inflate_huft *tl, inflate_huft *td, inflate_blocks_statef *s, z_streamp z);
int zlib_inflate_trees_dynamic(uInt nl, uInt nd, uIntf *c, uIntf *bl, uIntf *bd, inflate_huft **tl, inflate_huft **td, inflate_huft *hp, z_streamp z);
}

using zlibshim::exe_message;

// inffixed.h's tables (inftrees.c: global, as tpw_zlib is built with -Dlocal=).
extern "C" inflate_huft fixed_tl[512], fixed_td[32];

namespace {

const std::uintptr_t kExeFixedTl = 0x77a0a0, kExeFixedTd = 0x77b0a0;  // (the exe's fixed_tl / fixed_td)

// A pointer into the library's fixed_tl / fixed_td: the same entry of the exe's; any other value as it is.
std::uint32_t exe_fixed(std::uint32_t a) {
    const std::uintptr_t tl = reinterpret_cast<std::uintptr_t>(fixed_tl), td = reinterpret_cast<std::uintptr_t>(fixed_td);
    if (a - tl < sizeof fixed_tl) return static_cast<std::uint32_t>(kExeFixedTl + (a - tl));
    if (a - td < sizeof fixed_td) return static_cast<std::uint32_t>(kExeFixedTd + (a - td));
    return a;
}

// infcodes.c's inflate_codes_state (private to it; inflate_codes_new allocates 0x1c bytes): the fields that can point into a table.
struct CodesState {
    std::uint32_t mode, len;
    std::uint32_t tree;  // (sub.code.tree in LEN / DIST; a literal, or copy.get, in the other modes: never inside a table)
    std::uint32_t need;
    Byte lbits, dbits;
    std::uint32_t ltree, dtree;
};
static_assert(sizeof(CodesState) == 0x1c && offsetof(CodesState, ltree) == 0x14, "inflate_codes_state");

void exe_tables(void *codes) {
    CodesState *const c = static_cast<CodesState *>(codes);
    c->tree = exe_fixed(c->tree);
    c->ltree = exe_fixed(c->ltree);
    c->dtree = exe_fixed(c->dtree);
}

// The call in progress (one at a time on a thread: the shims never nest).
thread_local inflate_blocks_statef *t_blocks;
thread_local free_func t_zfree;

void __cdecl freeing(voidpf opaque, voidpf p) {
    if (t_blocks && t_blocks->mode == CODES && p == t_blocks->sub.decode.codes) exe_tables(p);
    t_zfree(opaque, p);
}

// Runs `run` (inflate_blocks on s, or inflate) with the exe's fixed tables and strings: see the top of the file.
template <class F> int with_exe_tables(inflate_blocks_statef *s, z_stream_s *z, F run) {
    const char *const before = z->msg;
    const free_func zfree = z->zfree;
    if (zfree) {
        t_blocks = s, t_zfree = zfree;
        z->zfree = freeing;
    }
    const int r = run();
    if (zfree) {
        z->zfree = zfree;
        t_blocks = nullptr, t_zfree = nullptr;
    }
    if (s && s->mode == CODES) exe_tables(s->sub.decode.codes);
    exe_message(z, before);
    return r;
}

}  // namespace

extern "C" {

int __cdecl inflate_trees_bits(unsigned *c, unsigned *bb, inflate_huft_s **tb, inflate_huft_s *hp, z_stream_s *z) {
    const char *const before = z->msg;
    const int r = zlib_inflate_trees_bits(c, bb, tb, hp, z);
    exe_message(z, before);
    return r;
}

int __cdecl inflate_trees_dynamic(unsigned nl, unsigned nd, unsigned *c, unsigned *bl, unsigned *bd, inflate_huft_s **tl, inflate_huft_s **td,
                                  inflate_huft_s *hp, z_stream_s *z) {
    const char *const before = z->msg;
    const int r = zlib_inflate_trees_dynamic(nl, nd, c, bl, bd, tl, td, hp, z);
    exe_message(z, before);
    return r;
}

int __cdecl inflate_blocks(inflate_blocks_state *s, z_stream_s *z, int r) {
    return with_exe_tables(s, z, [&] { return zlib_inflate_blocks(s, z, r); });
}

int __cdecl inflate_codes(inflate_blocks_state *s, z_stream_s *z, int r) {
    const char *const before = z->msg;
    r = zlib_inflate_codes(s, z, r);
    exe_message(z, before);
    return r;
}

int __cdecl inflate_fast(unsigned bl, unsigned bd, inflate_huft_s *tl, inflate_huft_s *td, inflate_blocks_state *s, z_stream_s *z) {
    const char *const before = z->msg;
    const int r = zlib_inflate_fast(bl, bd, tl, td, s, z);
    exe_message(z, before);
    return r;
}
}
