# libjpeg 6a's jcparam.c, the compression parameters (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, its names
# jpeg6a_*, the statics through the wrapper unit third_party/wrap/jpeg-6a/jcparam.c) against the exe's 6a copies. cinfo at CINFO
# (jpeg_compress_struct, 0x168 bytes; err +0 at ERR: error_exit a cdecl recorder, slot 0, msg_code +0x14, msg_parm +0x18; mem +4 at
# MEM: alloc_small ZALLOC over the case heap; global_state +0x10, CSTATE_START 100 mostly; comp_info +0x3c at COMP, room for 12
# components; quant_tbl_ptrs[4] +0x40, each null (a new table from jpeg_alloc_quant_table, the library's, alloc_small of pool 0: ZALLOC's
# zero items give a block of no size at the heap's next address, the same in both) or a table in QTBL; dc_huff_tbl_ptrs[4] +0x50 and
# ac_huff_tbl_ptrs[4] +0x60 into HTBL).
# jpeg_set_linear_quality passes the library's own copies of the two standard tables (the exe's at 0x705108 / 0x705208): read only.
# Mask (std_huff_tables, jpeg_set_defaults): add_huff_table copies each huffval whole, MEMCOPY of SIZEOF(huffval) = 256 bytes, from
# std_huff_tables' value lists of 12 (DC) and 162 (AC) bytes, so past the symbols a table holds the bytes after the list: the exe's
# neighbouring data there, the library's in the DLL. Those bytes (huffval[count..255], +0x11 + count .. +0x111 of each table) are
# masked. No code reads them: a table's symbol count is the sum of bits[1..16] (12, 162 here), and the compressor reads huffval only
# below it (jchuff.c's jpeg_make_c_derived_tbl, its loop p < lastp; jcmarker.c's emit_dht, i < length, the sum of bits; jchuff.c's
# jpeg_gen_optimal_table only writes it), and nothing else in the exe touches the tables. add_huff_table's own cases pass the bench's
# value list, the same 256 bytes to both, so they compare everything.
# Edges as the source meets them: jpeg_set_colorspace's JCS_UNKNOWN with more than 10 components reports JERR_COMPONENT_COUNT (a
# recorder that returns) and sets them all, past comp_info's 10 (COMP has room for 12, so input_components stays within 12).
# usage: jpeg6a_param.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=add_quant,linear,scaling,set_quality,set_defaults,std_huff,add_huff,default_cs,set_cs,progression,fill_a,fill_s,fill_dc)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, Mask, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); jcomapi.c's jpeg_alloc_quant_table and jpeg_alloc_huff_table
FUNCS = {'jpeg_add_quant_table': (0x64cd90, 0x64ce78), 'jpeg_set_linear_quality': (0x64ce78, 0x64ceb3),
         'jpeg_quality_scaling': (0x64ceb3, 0x64cef8), 'jpeg_set_quality': (0x64cef8, 0x64cf20), 'jpeg_set_defaults': (0x64cf20, 0x64d0a8),
         'std_huff_tables': (0x64d0a8, 0x64d121), 'add_huff_table': (0x64d121, 0x64d188), 'jpeg_default_colorspace': (0x64d188, 0x64d23b),
         'jpeg_set_colorspace': (0x64d23b, 0x64d859), 'jpeg_simple_progression': (0x64d859, 0x64dab2),
         'fill_a_scan': (0x64dab2, 0x64daf9), 'fill_scans': (0x64daf9, 0x64db5f), 'fill_dc_scans': (0x64db5f, 0x64dc00),
         'jpeg_alloc_quant_table': (0x64f43e, 0x64f46f), 'jpeg_alloc_huff_table': (0x64f46f, 0x64f4a0)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, QTBL, BASIC = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x800
COMP, HTBL, SCANS, BITS = RAM + 0x1000, RAM + 0x1400, RAM + 0x1a00, RAM + 0x1d00
K_ERROR = 0
QSIZE, HSIZE, CSIZE, SSIZE = 0x84, 0x118, 0x54, 0x24
STD_COUNTS = [12, 162, 12, 162]  # (std_huff_tables' dc[0], ac[0], dc[1], ac[1]: the symbols of each list)


def common(r, size=0x70, huff=False, heap_size=0x400):
    cinfo = bytearray(r.bytes(size))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, 0x10, 100 if r.random() < 0.9 else r.choice([0, 101, 102, r.getrandbits(32)]))
    for i in range(4):
        struct.pack_into('<I', cinfo, 0x40 + 4 * i, 0 if r.random() < 0.3 else QTBL + QSIZE * i)
    err = struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x3c)
    mem = struct.pack('<I', ZALLOC) + r.bytes(0x2c)
    regs = {CINFO: cinfo, ERR: err, MEM: mem, QTBL: bytearray(r.bytes(4 * QSIZE)), LOG: log_region(), HEAP: heap(heap_size)}
    if huff:  # dc[0], ac[0], dc[1], ac[1] (std_huff_tables' order) at HTBL, the others random
        for i, slot in enumerate((0x50, 0x60, 0x54, 0x64)):
            struct.pack_into('<I', cinfo, slot, HTBL + HSIZE * i)
        regs[HTBL] = bytearray(r.bytes(4 * HSIZE))
    return regs


def std_masks():
    return [Mask(HTBL, [HSIZE * i + 0x11 + o for i, n in enumerate(STD_COUNTS) for o in range(n, 256)],
                 "huffval past the symbols: copied from past the end of std_huff_tables' value lists, the exe's neighbouring data or the "
                 "library's; read only below the symbol count (jchuff.c's jpeg_make_c_derived_tbl, jcmarker.c's emit_dht)")]


def scale(r):
    return r.choice([0, 1, 50, 100, 200, 5000, r.randrange(-10, 300), r.randrange(5001), r.getrandbits(32)]) & 0xffffffff


def force(r):
    return r.choice([0, 1, 1, r.getrandbits(32)])


def add_quant(r):
    regs = common(r)
    basic = [r.choice([0, 1, 16, 99, 255, 256, r.randrange(256), r.randrange(1 << 16), r.getrandbits(32)]) for _ in range(64)]
    regs[BASIC] = struct.pack('<64I', *basic)
    return Call(0x64cd90, [CINFO, r.randrange(4), BASIC, scale(r), force(r)], regs)


def linear(r):
    return Call(0x64ce78, [CINFO, scale(r), force(r)], common(r))


def quality(r):
    return r.choice([-1, 0, 1, 2, 49, 50, 51, 75, 99, 100, 101, r.randrange(-200, 200), r.getrandbits(32)]) & 0xffffffff


def scaling(r):
    return Call(0x64ceb3, [quality(r)], {}, ret='a')


def set_quality(r):
    return Call(0x64cef8, [CINFO, quality(r), force(r)], common(r))


def colour(r, regs, null=False):
    """comp_info at COMP (null now and then with null, for jpeg_set_defaults: a block of no size at the heap), the colour spaces and component counts."""
    cinfo = regs[CINFO]
    struct.pack_into('<I', cinfo, 0x3c, 0 if null and r.random() < 0.05 else COMP)
    regs[COMP] = r.bytes(12 * CSIZE)
    struct.pack_into('<i', cinfo, 0x20, r.choice([1, 3, 4, r.randrange(-1, 13)]))  # input_components
    struct.pack_into('<I', cinfo, 0x24, r.randrange(6) if r.random() < 0.95 else r.choice([-1, 6, r.getrandbits(32)]) & 0xffffffff)  # in_color_space
    struct.pack_into('<I', cinfo, 0x30, r.choice([8, 8, 12, r.getrandbits(32)]) & 0xffffffff)  # data_precision (set to 8 by jpeg_set_defaults)


def set_defaults(r):
    regs = common(r, 0x168, huff=True, heap_size=0x800)
    colour(r, regs, null=True)
    return Call(0x64cf20, [CINFO], regs, masks=std_masks())


def std_huff(r):
    return Call(0x64d0a8, [CINFO], common(r, huff=True), masks=std_masks())


def add_huff(r):
    regs = common(r, huff=True)
    slot = r.choice([0x50, 0x54, 0x60, 0x64])  # (the tables common() lays out)
    if r.random() < 0.3:  # (a new table: jpeg_alloc_huff_table, a block of no size at the heap)
        struct.pack_into('<I', regs[CINFO], slot, 0)
    regs[BITS] = r.bytes(17 + 256)
    return Call(0x64d121, [CINFO, CINFO + slot, BITS, BITS + 17], regs)


def default_cs(r):
    regs = common(r, 0x168, heap_size=0x800)
    colour(r, regs)
    return Call(0x64d188, [CINFO], regs)


def set_cs(r):
    regs = common(r, 0x168, heap_size=0x800)
    colour(r, regs)
    cs = r.randrange(6) if r.random() < 0.95 else r.choice([-1, 6, r.getrandbits(32)])
    return Call(0x64d23b, [CINFO, cs], regs)


def progression(r):
    regs = common(r, 0x168, heap_size=0x1000)
    cinfo = regs[CINFO]
    n = r.choice([1, 2, 3, 3, 3, 4, 5, 10, 0, -1])
    struct.pack_into('<i', cinfo, 0x34, n)
    struct.pack_into('<I', cinfo, 0x38, 3 if r.random() < 0.5 else r.choice([1, 2, 4, 5, r.getrandbits(32)]) & 0xffffffff)  # jpeg_color_space
    return Call(0x64d859, [CINFO], regs)


def ints(r, n):
    return [r.choice([0, 1, 2, 5, 6, 63, r.randrange(64), r.getrandbits(32)]) for _ in range(n)]


def fill_a(r):
    return Call(0x64dab2, [SCANS + SSIZE * r.randrange(4)] + ints(r, 5), {SCANS: r.bytes(16 * SSIZE)}, ret='a')


def fill_s(r):
    return Call(0x64daf9, [SCANS, r.choice([0, 1, 2, 3, 4, 5, 10, r.randrange(-2, 12)]) & 0xffffffff] + ints(r, 4),
                {SCANS: r.bytes(16 * SSIZE)}, ret='a')


def fill_dc(r):
    return Call(0x64db5f, [SCANS, r.choice([0, 1, 2, 3, 4, 5, 10, r.randrange(-2, 12)]) & 0xffffffff] + ints(r, 2),
                {SCANS: r.bytes(16 * SSIZE)}, ret='a')


OPS = [f for f in (add_quant, linear, scaling, set_quality, set_defaults, std_huff, add_huff, default_cs, set_cs, progression, fill_a,
                   fill_s, fill_dc) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
