# libjpeg 6b's jdmerge.c, the merged upsampler (h2v1 / h2v2 chroma upsampling and YCbCr -> RGB in one pass; src/game/lib/jpeg6b: the
# library built from third_party/jpeg-6b, the module through its wrapper unit third_party/wrap/jpeg-6b/jdmerge.c) against the exe's
# copies. cinfo at CINFO (output_width +0x5c, output_height +0x60, sample_range_limit +0x120 into a random range-limit table at RANGE,
# upsample +0x1a0 at UPS: my_upsampler, its four tables at TABLES (build_ycc_rgb_table's values, or perturbed), upmethod +0xc,
# spare_row +0x20, spare_full +0x24 (a byte: boolean is an unsigned char), out_row_width +0x28, rows_to_go +0x2c).
# merged_2v_upsample and merged_1v_upsample call upmethod, a cdecl recorder here; merged_2v hands it work_ptrs on its stack, so that
# argument is logged as a hash of its two row pointers (blob), and merged_1v's output_buf + *out_row_ctr the same way. merged_2v also
# calls jcopy_sample_rows (the spare row out). build_ycc_rgb_table allocates through cinfo->mem->alloc_small, fncmp's ZALLOC stub over
# the case heap (its (opaque, items, size) is alloc_small's (cinfo, JPOOL_IMAGE = 1, size)).
# jinit_merged_upsampler (0x646ee0) is the library's through jpeg6b.cpp's shim, which puts the exe's method addresses in place of the
# library's (upsample, start_pass, upmethod): the bench compares them as stored. alloc_small and alloc_large are both ZALLOC; it reads
# output_width, out_color_components +0x64 and max_v_samp_factor +0x114 (2: h2v2 with a spare row, anything else h2v1).
# usage: jpeg6b_merge.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=h2v1,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, blob, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'build_ycc_rgb_table': (0x646f70, 0x647040), 'start_pass_merged_upsample': (0x647040, 0x647060),
    'merged_2v_upsample': (0x647060, 0x647130), 'merged_1v_upsample': (0x647130, 0x647170),
    'h2v1_merged_upsample': (0x647170, 0x6472f0), 'h2v2_merged_upsample': (0x6472f0, 0x6474f0),
}
FUNCS['jinit_merged_upsampler'] = (0x646ee0, 0x646f70)
B = Bench(sorted(list(FUNCS.values()) + [(0x6428e0, 0x642930)]))  # (and jcopy_sample_rows, which merged_2v_upsample calls)

CINFO, MEM, UPS, IMAGE, CTRS, COMP_ROWS, OUT_ROWS, RANGE, TABLES = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x480, RAM + 0x500,
                                                                    RAM + 0x800, RAM + 0x1000, RAM + 0x2000)
IN, OUT, SPARE = RAM + 0x10000, RAM + 0x20000, RAM + 0x30000
MEM_, OUTPUT_WIDTH, OUTPUT_HEIGHT, SAMPLE_RANGE_LIMIT, UPSAMPLE = 0x4, 0x5c, 0x60, 0x120, 0x1a0
UPMETHOD, SPARE_ROW, SPARE_FULL, OUT_ROW_WIDTH, ROWS_TO_GO = 0xc, 0x20, 0x24, 0x28, 0x2c
RANGE_SIZE = 5 * 256 + 128  # (jdmaster.c's prepare_range_limit_table; sample_range_limit is 256 bytes in)
SCALEBITS, ONE_HALF = 16, 1 << 15
K_UP = 0  # upmethod's recorder slot


def fix(x):
    return int(x * (1 << SCALEBITS) + 0.5)


def tables(r):
    """build_ycc_rgb_table's Cr_r_tab, Cb_b_tab, Cr_g_tab, Cb_g_tab, or perturbed within what range_limit can take."""
    xs = [i - 128 for i in range(256)]
    crr, cbb = [(fix(1.40200) * x + ONE_HALF) >> SCALEBITS for x in xs], [(fix(1.77200) * x + ONE_HALF) >> SCALEBITS for x in xs]
    crg, cbg = [-fix(0.71414) * x for x in xs], [-fix(0.34414) * x + ONE_HALF for x in xs]
    if r.random() < 0.3:
        for t in (crr, cbb):
            for _ in range(r.randrange(1, 20)):
                t[r.randrange(256)] = r.randrange(-256, 384)
        for t in (crg, cbg):
            for _ in range(r.randrange(1, 20)):
                t[r.randrange(256)] = r.randrange(-128 << 16, 128 << 16)
    return struct.pack('<256i', *crr) + struct.pack('<256i', *cbb) + struct.pack('<256i', *crg) + struct.pack('<256i', *cbg)


def upsampler(r, out_row_width):
    ups = bytearray(r.bytes(0x30))
    struct.pack_into('<5I', ups, UPMETHOD, recorder(K_UP, 4, cdecl=True), TABLES, TABLES + 0x400, TABLES + 0x800, TABLES + 0xc00)
    struct.pack_into('<I', ups, SPARE_ROW, SPARE)
    ups[SPARE_FULL] = r.choice([0, 0, 1, r.randrange(256)])
    struct.pack_into('<2I', ups, OUT_ROW_WIDTH, out_row_width, r.choice([0, 1, 2, 3, r.randrange(100)]))
    return ups


def image(r, rows0, rows12, width):
    """input_buf: component 0's rows0 rows of width samples, components 1 and 2's rows12 rows of (width + 1) // 2 (with slack)."""
    stride = width + 1 + r.randrange(4)
    counts = (rows0, rows12, rows12)
    starts = [0, rows0, rows0 + rows12]
    comp_rows = b''.join(struct.pack('<I', IN + (starts[c] + i) * stride) for c in range(3) for i in range(counts[c]))
    return {IMAGE: struct.pack('<3I', *(COMP_ROWS + 4 * s for s in starts)), COMP_ROWS: comp_rows or r.bytes(4),
            IN: r.bytes((rows0 + 2 * rows12) * stride + 1)}


def common(r, width):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, OUTPUT_WIDTH, width)
    struct.pack_into('<I', cinfo, SAMPLE_RANGE_LIMIT, RANGE + 256)
    struct.pack_into('<I', cinfo, UPSAMPLE, UPS)
    return {CINFO: cinfo, UPS: upsampler(r, width * 3), TABLES: tables(r), RANGE: r.bytes(RANGE_SIZE)}


def out_rows(r, n, size):
    stride = size + r.randrange(4)
    return {OUT_ROWS: struct.pack('<%dI' % n, *(OUT + i * stride for i in range(n))), OUT: r.bytes(n * stride + 1)}


def width_of(r):
    return r.choice([1, 2, 3, 7, 8, r.randrange(1, 100)])


def h2v1(r):
    width, ctr = width_of(r), r.randrange(3)
    regions = common(r, width)
    regions.update(image(r, ctr + 1, ctr + 1, width))
    regions.update(out_rows(r, 1, width * 3))
    return Call(0x647170, [CINFO, IMAGE, ctr, OUT_ROWS], regions)


def h2v2(r):
    width, ctr = width_of(r), r.randrange(3)
    regions = common(r, width)
    regions.update(image(r, 2 * ctr + 2, ctr + 1, width))
    regions.update(out_rows(r, 2, width * 3))
    return Call(0x6472f0, [CINFO, IMAGE, ctr, OUT_ROWS], regions)


def merged(address):
    def op(r):
        width = width_of(r)
        regions = common(r, width)
        regions.update(image(r, 1, 1, width))
        in_ctr, out_ctr = r.randrange(5), r.randrange(4)
        out_avail = out_ctr + r.choice([0, 1, 1, 2, 2, 3])
        regions.update(out_rows(r, out_ctr + 3, width * 3))
        regions[SPARE] = r.bytes(width * 3 + 1)
        regions[CTRS] = struct.pack('<2I', in_ctr, out_ctr)
        log = log_region()
        blob(log, K_UP, 3, 0b11)  # (merged_2v's work_ptrs on its stack; merged_1v's output_buf + *out_row_ctr)
        regions[LOG] = log
        return Call(address, [CINFO, IMAGE, CTRS, r.randrange(4), OUT_ROWS, CTRS + 4, out_avail], regions)
    op.__name__ = {0x647060: 'merged_2v', 0x647130: 'merged_1v'}[address]
    return op


def start_pass(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, UPSAMPLE, UPS)
    return Call(0x647040, [CINFO], {CINFO: cinfo, UPS: r.bytes(0x30)})


def build(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, MEM_, MEM)
    struct.pack_into('<I', cinfo, UPSAMPLE, UPS)
    return Call(0x646f70, [CINFO], {CINFO: cinfo, MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), UPS: r.bytes(0x30), HEAP: heap(0x1100)})


def jinit(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, MEM_, MEM)
    struct.pack_into('<I', cinfo, OUTPUT_WIDTH, width_of(r))
    struct.pack_into('<i', cinfo, 0x64, r.choice([1, 3, 3, 4, r.randrange(0, 6)]))  # out_color_components
    struct.pack_into('<i', cinfo, 0x114, r.choice([1, 2, 2, r.randrange(0, 5)]))     # max_v_samp_factor
    return Call(0x646ee0, [CINFO], {CINFO: cinfo, MEM: struct.pack('<2I', ZALLOC, ZALLOC) + r.bytes(0x28), HEAP: heap(0x1800)})


OPS = [f for f in (build, jinit, start_pass, h2v1, h2v2, merged(0x647060), merged(0x647130))
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
