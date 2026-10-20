# libjpeg 6b's jquant2.c, the two-pass colour quantizer (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the module
# through its wrapper unit third_party/wrap/jpeg-6b/jquant2.c) against the exe's copies. cinfo at CINFO (err +0 at ERR: error_exit a
# cdecl recorder (slot 0), emit_message +4 (slot 1), msg_code +0x14 / msg_parm +0x18; mem +4 at MEM: alloc_small and alloc_large
# ZALLOC over the case heap, alloc_sarray +8 a cdecl recorder (slot 2) answering SARRAY (three rows of 256 at CMAP_ROWS); dither_mode
# +0x4c, desired_number_of_colors +0x54, enable_2pass_quant +0x5a, output_width +0x5c, out_color_components +0x64,
# actual_number_of_colors +0x70 (0..256; 257 only for start_pass_2_quant, which checks it: find_nearby_colors' mindist[] has 256), colormap +0x74 at CMAPP (the same three rows), sample_range_limit +0x120 256 bytes into a
# random table at RANGE, cquantize +0x1a8 at CQ (my_cquantizer, 0x2c bytes: sv_colormap +0x10 at CMAPP, desired +0x14, histogram +0x18
# at HISTP, needs_zeroed +0x1c, fserrors +0x20 at FSERR, on_odd_row +0x24, error_limiter +0x28 255 ints into ELIMIT)).
# The histogram's 32 hist2d pointers share four 0x1000-byte blocks at HIST (the code only indexes them): as pass 1 leaves it (counts,
# many zero) or as pass 2's inverse-colormap cache (0, or a colour + 1 up to actual_number_of_colors). The pass-2 dither's errors are
# within +-2000 and the limiter's within +-255, so a pixel's sum stays inside the range-limit table, as the exe's own tables keep it.
# Boxes (0x20 bytes: c0min..c2max, volume, colorcount) at BOXES. find_biggest_color_pop (0x6486e0) stays the exe's (the hand port
# Records32_MaxWeight; median_cut's exe copy calls it, so the Bench's ranges have it). compute_color's division by an empty box's zero
# total is not dealt (both fault). select_colors and finish_pass1 scan the whole 32x64x32 histogram several times (seconds a case in
# the emulator), so they are dealt one case in 100, with 1..4 colours.
# usage: jpeg6b_quant2.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,prescan,volume,update,median,compute,select,
#        nearby,best,fill,pass2,pass2_fs,error_limit,finish1,start,new_map)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); find_biggest_color_pop (the exe's) and jutils.c's jzero_far
FUNCS = {'jinit_2pass_quantizer': (0x6474f0, 0x647620), 'init_error_limit': (0x647620, 0x6476c0), 'start_pass_2_quant': (0x6476c0, 0x6477e0),
         'prescan_quantize': (0x6477e0, 0x647860), 'pass2_no_dither': (0x647860, 0x647930), 'fill_inverse_cmap': (0x647930, 0x647a50),
         'find_nearby_colors': (0x647a50, 0x647c50), 'find_best_colors': (0x647c50, 0x647db0), 'pass2_fs_dither': (0x647db0, 0x6480a0),
         'finish_pass1': (0x6480a0, 0x6480d0), 'select_colors': (0x6480d0, 0x648160), 'update_box': (0x648160, 0x6485a0),
         'median_cut': (0x6485a0, 0x6486e0), 'find_biggest_color_pop': (0x6486e0, 0x648720), 'find_biggest_volume': (0x648720, 0x648750),
         'compute_color': (0x648750, 0x6488e0), 'new_color_map_2_quant': (0x6488e0, 0x6488f0), 'jzero_far': (0x642960, 0x642980)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, CQ, SARRAY, CMAPP, HISTP, BOXES = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x340, RAM + 0x380, RAM + 0x3a0, RAM + 0x400, RAM + 0x500
COLORLIST, BEST, IN_PTRS, OUT_PTRS = RAM + 0x900, RAM + 0xa00, RAM + 0xb00, RAM + 0xb40
ELIMIT, CMAP_ROWS, FSERR, RANGE, IN, OUT, HIST = (RAM + 0x1000, RAM + 0x1800, RAM + 0x2000, RAM + 0x2400, RAM + 0x3000, RAM + 0x4000,
                                                  RAM + 0x10000)
K_ERROR, K_EMIT, K_SARRAY = 0, 1, 2
IN_ROW, OUT_ROW, RANGE_SIZE, BOX = 0x200, 0x80, 5 * 256 + 128, 0x20


def common(r, cache=False, heap_size=0x3000, wide=False):
    log = log_region()
    set_return(log, K_SARRAY, SARRAY)
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<i', cinfo, 0x4c, r.choice([0, 1, 2]) if r.random() < 0.95 else 3)
    struct.pack_into('<i', cinfo, 0x54, r.choice([8, 16, 64, 216, 256, r.randrange(8, 257)]) if r.random() < 0.9 else r.choice([1, 7, 257]))
    cinfo[0x5a] = r.choice([0, 1, 1])
    width = r.choice([0, 1, 2, 7, 16, 17, r.randrange(1, 101)])
    struct.pack_into('<I', cinfo, 0x5c, width)
    struct.pack_into('<i', cinfo, 0x64, 3 if r.random() < 0.95 else r.choice([1, 4]))
    ncolors = r.choice([1, 2, 8, 64, 255, 256, r.randrange(1, 257)]) if r.random() < 0.95 else r.choice([0, 257] if wide else [0, 1])
    if cache:  # (pass 2 has a colour at least: with none, find_best_colors leaves its local bestcolor[] unset, and that goes in the cache)
        ncolors = max(ncolors, 1)
    struct.pack_into('<iI', cinfo, 0x70, ncolors, CMAPP)
    struct.pack_into('<I', cinfo, 0x120, RANGE + 256)
    struct.pack_into('<I', cinfo, 0x1a8, CQ)
    cq = bytearray(r.bytes(0x2c))
    struct.pack_into('<IiI', cq, 0x10, CMAPP, r.randrange(1, 257), HISTP)
    cq[0x1c] = r.choice([0, 1])
    struct.pack_into('<I', cq, 0x20, FSERR if r.random() < 0.8 else 0)
    cq[0x24] = r.choice([0, 1])
    struct.pack_into('<I', cq, 0x28, ELIMIT + 255 * 4 if r.random() < 0.8 else 0)
    if cache:
        hist = b''.join(struct.pack('<H', 0 if r.random() < 0.4 else r.randrange(1, max(min(ncolors, 256), 1) + 1)) for _ in range(4 * 0x800))
    else:
        hist = b''.join(struct.pack('<H', 0 if r.random() < 0.5 else r.choice([1, 2, 3, r.randrange(1, 300), r.randrange(0x10000)]))
                        for _ in range(4 * 0x800))
    rows = struct.pack('<3I', *[CMAP_ROWS + 0x100 * i for i in range(3)])
    return {CINFO: cinfo, LOG: log, HEAP: heap(heap_size),
            ERR: struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x78),
            MEM: struct.pack('<III', ZALLOC, ZALLOC, recorder(K_SARRAY, 4, cdecl=True)) + r.bytes(0x24),
            CQ: cq, SARRAY: rows, CMAPP: rows, CMAP_ROWS: r.bytes(0x300),
            HISTP: struct.pack('<32I', *[HIST + 0x1000 * r.randrange(4) for _ in range(32)]), HIST: hist,
            ELIMIT: b''.join(struct.pack('<i', r.randrange(-255, 256)) for _ in range(511)),
            FSERR: b''.join(struct.pack('<h', r.randrange(-2000, 2001)) for _ in range(3 * 102)), RANGE: r.bytes(RANGE_SIZE)}


def a_box(r, full=False):
    """c0min..c2max within the histogram (sometimes the whole of it, sometimes a single cell), a volume and a colour count."""
    def span(top):
        if full or r.random() < 0.1:
            return 0, top
        lo = r.randrange(top + 1)
        return lo, min(top, lo + r.choice([0, 1, 2, 3, 7, r.randrange(top + 1)]))
    c0, c1, c2 = span(31), span(63), span(31)
    return struct.pack('<6iii', c0[0], c0[1], c1[0], c1[1], c2[0], c2[1], r.choice([0, 1, r.randrange(-10, 100000)]),
                       r.choice([0, 1, r.randrange(-10, 100000)]))


def boxes(r, regions, n):
    regions[BOXES] = b''.join(a_box(r) for _ in range(n)) + r.bytes(0x400 - BOX * n)


def jinit(r):
    return Call(0x6474f0, [CINFO], common(r, heap_size=0x23000))


def rows(r, regions, address):
    width = struct.unpack_from('<I', regions[CINFO], 0x5c)[0]
    n = r.choice([0, 1, 1, 2, 3, r.randrange(1, 9)])
    regions[IN_PTRS] = struct.pack('<8I', *[IN + IN_ROW * i for i in range(8)])
    regions[OUT_PTRS] = struct.pack('<8I', *[OUT + OUT_ROW * i for i in range(8)])
    regions[IN], regions[OUT] = r.bytes(8 * IN_ROW), r.bytes(8 * OUT_ROW)
    assert width * 3 <= IN_ROW and width <= OUT_ROW
    return Call(address, [CINFO, IN_PTRS, OUT_PTRS, n], regions)


def prescan(r):
    regions = common(r)
    if r.random() < 0.3:  # (cells at the count's top, which prescan_quantize keeps from wrapping, and just under it)
        regions[HIST] = b''.join(struct.pack('<H', r.choice([0xffff, 0xffff, 0xfffe, r.randrange(0x10000)])) for _ in range(4 * 0x800))
    return rows(r, regions, 0x6477e0)


def pass2(r):
    return rows(r, common(r, cache=True), 0x647860)


def pass2_fs(r):
    return rows(r, common(r, cache=True), 0x647db0)


def volume(r):
    regions = common(r)
    n = r.choice([1, 2, 5, r.randrange(0, 33)])
    boxes(r, regions, n)
    return Call(0x648720, [BOXES, n], regions, ret='a')


def update(r):
    regions = common(r)
    boxes(r, regions, 1)
    if r.random() < 0.1:
        regions[BOXES] = a_box(r, True) + regions[BOXES][BOX:]
    return Call(0x648160, [CINFO, BOXES], regions)


def median(r):
    regions = common(r)
    n = r.choice([1, 1, 2, 3])
    boxes(r, regions, n)
    return Call(0x6485a0, [CINFO, BOXES, n, r.choice([n, n + 1, 4, 8, 16, 32])], regions, ret='a')


def nonempty(r, regions):
    """a box whose histogram cells are not all zero (compute_color divides by their total)."""
    hist = bytearray(regions[HIST])
    box = a_box(r)
    c0, _, c1, _, c2 = struct.unpack_from('<5i', box)
    block = struct.unpack_from('<32I', regions[HISTP])[c0] - HIST
    struct.pack_into('<H', hist, block + (c1 * 32 + c2) * 2, r.randrange(1, 300))
    regions[HIST] = bytes(hist)
    return box


def compute(r):
    regions = common(r)
    regions[BOXES] = nonempty(r, regions) + r.bytes(0x400 - BOX)
    return Call(0x648750, [CINFO, BOXES, r.randrange(256)], regions)


def select(r):  # (few colours: see the header)
    return Call(0x6480d0, [CINFO, r.choice([1, 2, 3, 4])], common(r))


def corner(r):
    """a cell box's lower corner as fill_inverse_cmap gives it (the box index << 5 plus half a cell), or anywhere."""
    if r.random() < 0.8:
        return [(r.randrange(8) << 5) + h for h in (4, 2, 4)]
    return [r.randrange(-64, 300) for _ in range(3)]


def nearby(r):
    regions = common(r)
    regions[COLORLIST] = r.bytes(0x100)
    at = corner(r)
    if r.random() < 0.3:  # (a tie: a colour at the box's lower corner and its reflection through the upper one, whose mindist is the
        k = [r.randrange(7) for _ in range(3)]  # first's maxdist, the minmaxdist; find_nearby_colors keeps a colour whose mindist equals it)
        at = [(k[0] << 5) + 4, (k[1] << 5) + 2, (k[2] << 5) + 4]
        far = [at[0] + 48, at[1] + 56, at[2] + 48]
        pair = [at, far] if r.random() < 0.5 else [far, at]
        cmap = bytearray(regions[CMAP_ROWS])
        for i, colour in enumerate(pair):
            for c in range(3):
                cmap[0x100 * c + i] = colour[c]
        regions[CMAP_ROWS] = bytes(cmap)
        struct.pack_into('<i', regions[CINFO], 0x70, r.choice([2, 2, 3]))
    return Call(0x647a50, [CINFO] + at + [COLORLIST], regions, ret='a')


def best(r):
    regions = common(r)
    ncolors = struct.unpack_from('<i', regions[CINFO], 0x70)[0]
    n = r.choice([0, 1, 2, 5, r.randrange(1, 257)])
    regions[COLORLIST] = bytes(r.randrange(max(min(ncolors, 256), 1)) for _ in range(n)) + r.bytes(0x100 - n)
    regions[BEST] = r.bytes(0x80)
    return Call(0x647c50, [CINFO] + corner(r) + [n, COLORLIST, BEST], regions)


def fill(r):
    return Call(0x647930, [CINFO, r.randrange(32), r.randrange(64), r.randrange(32)], common(r, cache=True))


def error_limit(r):
    return Call(0x647620, [CINFO], common(r))


def finish1(r):  # (few colours: see the header)
    regions = common(r)
    struct.pack_into('<i', regions[CQ], 0x14, r.choice([1, 2, 3, 4]))
    return Call(0x6480a0, [CINFO], regions)


def start(r):
    return Call(0x6476c0, [CINFO, r.choice([0, 1])], common(r, wide=True))


def new_map(r):
    return Call(0x6488e0, [CINFO], common(r))


OPS = [f for f in (jinit, prescan, volume, update, median, compute, select, nearby, best, fill, pass2, pass2_fs, error_limit, finish1,
                   start, new_map)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


WHOLE = [f for f in OPS if f in (select, finish1)]  # (dealt one case in 100: see the header)
PARTS = [f for f in OPS if f not in WHOLE] or WHOLE


def case(r):
    return r.choice(WHOLE)(r) if WHOLE and r.random() < 0.01 else r.deal(PARTS)(r)


if __name__ == '__main__':
    B.main(case)
