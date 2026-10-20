/* zlib 1.1.3's functions that leave one of the library's own addresses where the exe's code also looks (a static descriptor,
   inffixed.h's tables kept in a codes state, a strm->msg string, adler32 as the check function), built under zlib_* names. src/game/lib/zlib.cpp
   defines the library's names as shims that call these and put the exe's copy of the same table or string back, so the hook
   table points the exe's addresses at the shims. Force-included when tpw_zlib compiles; the library's own calls between these
   go to the renamed functions directly. See docs/porting.md, "Library functions". */

#define _tr_init zlib__tr_init
#define deflateReset zlib_deflateReset
#define deflateInit2_ zlib_deflateInit2_
#define deflate zlib_deflate
#define inflateInit2_ zlib_inflateInit2_
#define inflate zlib_inflate
#define inflate_blocks zlib_inflate_blocks
#define inflate_codes zlib_inflate_codes
#define inflate_fast zlib_inflate_fast
#define inflate_trees_bits zlib_inflate_trees_bits
#define inflate_trees_dynamic zlib_inflate_trees_dynamic
