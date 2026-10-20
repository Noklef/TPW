// zlib 1.1.3's inflate side: see zlib.h. The inflate layouts the exe's copies share with the library, and the inflate shims: the
// library's function (built as zlib_<name>, third_party/config/zlib_shims.h), then the exe's copy of a string or table put back where
// the library left its own address.
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
