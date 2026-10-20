// zlib 1.1.3 as the game links it: the exe's copies (0x6351a0..0x64bf70, re/libmap.csv) replaced by the
// library built from third_party/zlib-1.1.3 (tpw_zlib), unmodified. See docs/porting.md, "Library functions".
//
// The hook table points the exe's addresses at these. Most are zlib's own `local` (static) functions:
// tpw_zlib is built with -Dlocal= (zlib's documented switch, zutil.h) so they link by name. The
// declarations use the library's own types by their struct tags only, so this header pulls in none of
// zlib's macros (deflate.h defines Len, Code, Freq; zutil.h defines local). All are cdecl, as in the exe.
#pragma once

extern "C" {

struct internal_state;        // deflate_state (deflate.h), 0x16b8 bytes
struct ct_data_s;             // ct_data: {freq | code, dad | len} (u16, u16)
struct tree_desc_s;           // tree_desc: {dyn_tree, max_code, stat_desc}
struct inflate_huft_s;        // inflate_huft (inftrees.h), 8 bytes
struct z_stream_s;            // z_stream (zlib.h), 0x38 bytes

// adler32.c
// 0x63ea00: the Adler-32 checksum of len bytes at buf, continuing adler (1 for a null buf).
unsigned long __cdecl adler32(unsigned long adler, const unsigned char *buf, unsigned int len);

// deflate.c
// 0x635710: two bytes into the pending buffer, most significant first.
void __cdecl putShortMSB(internal_state *s, unsigned int b);
// 0x635740: as much of the pending output as fits into next_out.
void __cdecl flush_pending(z_stream_s *strm);
// 0x635870: a new stream's matching state (the level's parameters from configuration_table, the hash heads cleared).
void __cdecl lm_init(internal_state *s);
// 0x635910 / 0x635c20 / 0x636100: compress what fits (stored; fast, no lazy matching; lazy matching); a block_state.
int __cdecl deflate_stored(internal_state *s, int flush);
int __cdecl deflate_fast(internal_state *s, int flush);
int __cdecl deflate_slow(internal_state *s, int flush);
// 0x635a70: refills the window from the input, sliding it down (and the hash chains) when it is full.
void __cdecl fill_window(internal_state *s);
// 0x635ba0: up to size bytes of input into buf (with the adler32 of them unless noheader); the count.
int __cdecl read_buf(z_stream_s *strm, unsigned char *buf, unsigned size);
// 0x635f80: the longest match at strstart along the hash chain from cur_match (match_start set); its length.
unsigned int __cdecl longest_match(internal_state *s, unsigned cur_match);

// trees.c
// 0x63eba0: zeroes the three trees' frequencies (END_BLOCK's is 1) and the block's counters.
void __cdecl init_block(internal_state *s);
// 0x63ec10: sends a stored block (3 header bits, then the bytes after the length and its complement).
void __cdecl _tr_stored_block(internal_state *s, char *buf, unsigned long stored_len, int eof);
// 0x63ecb0: sends an empty static block (and another if the last EOB left fewer than 9 bits lookahead).
void __cdecl _tr_align(internal_state *s);
// 0x63ef00: ends the block: the cheapest of stored, static and dynamic trees; init_block; bi_windup at eof.
void __cdecl _tr_flush_block(internal_state *s, char *buf, unsigned long stored_len, int eof);
// 0x63f100: builds desc's Huffman tree from its frequencies (codes and lengths, opt_len and static_len).
void __cdecl build_tree(internal_state *s, tree_desc_s *desc);
// 0x63f340: sifts heap entry k down (smaller frequency, then smaller depth, first).
void __cdecl pqdownheap(internal_state *s, ct_data_s *tree, int k);
// 0x63f400: the optimal bit lengths for the tree under construction, limited to max_length.
void __cdecl gen_bitlen(internal_state *s, tree_desc_s *desc);
// 0x63f630: the codes for the lengths in tree[0..max_code] from bl_count (bits reversed).
void __cdecl gen_codes(ct_data_s *tree, int max_code, unsigned short *bl_count);
// 0x63f6b0: the bit length tree of both trees; the index of the last bit length code sent.
int __cdecl build_bl_tree(internal_state *s);
// 0x63f730: counts tree's code lengths (runs as REP_3_6 / REPZ_3_10 / REPZ_11_138) into bl_tree.
void __cdecl scan_tree(internal_state *s, ct_data_s *tree, int max_code);
// 0x63f820: sends the header of a dynamic block: the counts, the bit length codes, both trees.
void __cdecl send_all_trees(internal_state *s, int lcodes, int dcodes, int blcodes);
// 0x63fa90: sends tree's code lengths with the bit length codes.
void __cdecl send_tree(internal_state *s, ct_data_s *tree, int max_code);
// 0x640010: sends the block's literals and matches with the given trees, then END_BLOCK.
void __cdecl compress_block(internal_state *s, ct_data_s *ltree, ct_data_s *dtree);
// 0x640460: data_type: binary (0) or ASCII (1), from the literals' frequencies.
void __cdecl set_data_type(internal_state *s);
// 0x6404d0: the low len bits of code in reverse order.
unsigned __cdecl bi_reverse(unsigned code, int len);
// 0x6404f0: writes out bi_buf's full bytes (16 bits as a short, 8 or more as a byte).
void __cdecl bi_flush(internal_state *s);
// 0x640580: writes out the rest of bi_buf, byte aligned.
void __cdecl bi_windup(internal_state *s);
// 0x6405f0: bi_windup, then len bytes of buf (after len and ~len as shorts when header).
void __cdecl copy_block(internal_state *s, char *buf, unsigned len, int header);

// inftrees.c
// 0x649e10: the decoding tables (from hp + *hn) for n codes of lengths b; values below s are themselves, the others d[] with e[]
// extra bits; *t the first table, *m its bits; Z_OK, Z_BUF_ERROR (incomplete), Z_DATA_ERROR (oversubscribed) or Z_MEM_ERROR.
int __cdecl huft_build(unsigned *b, unsigned n, unsigned s, const unsigned *d, const unsigned *e, inflate_huft_s **t, unsigned *m,
                       inflate_huft_s *hp, unsigned *hn, unsigned *v);

// The shims: the library's function (built as zlib_<name>, third_party/config/zlib_shims.h), then the exe's copy put back where it
// left one of its own addresses (src/game/lib/zlib.cpp).
// 0x649d60: the bit length codes' table (19 lengths in c); z->msg on a bad set (the exe's string).
int __cdecl inflate_trees_bits(unsigned *c, unsigned *bb, inflate_huft_s **tb, inflate_huft_s *hp, z_stream_s *z);
// 0x64a2c0: the literal / length and distance tables of a dynamic block (nl + nd lengths in c); z->msg on a bad set (the exe's string).
int __cdecl inflate_trees_dynamic(unsigned nl, unsigned nd, unsigned *c, unsigned *bl, unsigned *bd, inflate_huft_s **tl, inflate_huft_s **td,
                                  inflate_huft_s *hp, z_stream_s *z);
}

// What the shims share (src/game/lib/zlib.cpp).
namespace zlibshim {
// The call stored a message in z->msg (it is no longer `before`): the exe's copy of the same text instead of the library's.
void exe_message(z_stream_s *z, const char *before);
}
