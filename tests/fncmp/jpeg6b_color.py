# libjpeg 6b's jdcolor.c, the output colour space converters (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the
# module through its wrapper unit third_party/wrap/jpeg-6b/jdcolor.c) against the exe's copies. cinfo at CINFO (output_width +0x5c,
# num_components +0x24, sample_range_limit +0x120 into a random range-limit table at RANGE, cconvert +0x1a4 at CC with its four
# tables at TABLES: build_ycc_rgb_table's values, or perturbed), the input image (an array of components' row arrays at IMAGE) and the
# output rows at OUT_ROWS. build_ycc_rgb_table allocates its tables through cinfo->mem->alloc_small, here fncmp's ZALLOC stub over
# the case heap (its (opaque, items, size) is alloc_small's (cinfo, JPOOL_IMAGE = 1, size)).
# ycck_cmyk_convert (0x646d70) stays the exe's for now: the listing has no code from 0x646d62 to 0x646dc9 (Ghidra never reached its
# head), so the emulator cannot run it until the listing is dumped again with a function there (re/listing_added.txt).
# usage: jpeg6b_color.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=ycc_rgb,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, ZALLOC, Bench, Call, heap  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'build_ycc_rgb_table': (0x646a30, 0x646b00), 'ycc_rgb_convert': (0x646b00, 0x646c40), 'null_convert': (0x646c40, 0x646cd0),
    'grayscale_convert': (0x646cd0, 0x646d00), 'gray_rgb_convert': (0x646d00, 0x646d70),
}
B = Bench(sorted(list(FUNCS.values()) + [(0x6428e0, 0x642930)]))  # (and jcopy_sample_rows, which grayscale_convert calls)

CINFO, MEM, CC, IMAGE, COMP_ROWS, OUT_ROWS, RANGE, TABLES = (RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x500, RAM + 0x800,
                                                             RAM + 0x1000, RAM + 0x2000)
IN, OUT = RAM + 0x10000, RAM + 0x20000
MEM_, NUM_COMPONENTS, OUTPUT_WIDTH, SAMPLE_RANGE_LIMIT, CCONVERT = 0x4, 0x24, 0x5c, 0x120, 0x1a4
RANGE_SIZE = 5 * 256 + 128  # (jdmaster.c's prepare_range_limit_table; sample_range_limit is 256 bytes in)
SCALEBITS, ONE_HALF = 16, 1 << 15


def fix(x):
    return int(x * (1 << SCALEBITS) + 0.5)


def ycc_tables():
    """build_ycc_rgb_table's Cr_r_tab, Cb_b_tab, Cr_g_tab, Cb_g_tab."""
    xs = [i - 128 for i in range(256)]
    return ([(fix(1.40200) * x + ONE_HALF) >> SCALEBITS for x in xs], [(fix(1.77200) * x + ONE_HALF) >> SCALEBITS for x in xs],
            [-fix(0.71414) * x for x in xs], [-fix(0.34414) * x + ONE_HALF for x in xs])


YCC = ycc_tables()


def tables(r):
    """The four tables, as build_ycc_rgb_table makes them or perturbed within what range_limit can take (y plus a red or blue entry,
    or y plus the green sum >> 16, from -256 to past 640)."""
    crr, cbb, crg, cbg = (list(t) for t in YCC)
    if r.random() < 0.3:
        for t in (crr, cbb):
            for _ in range(r.randrange(1, 20)):
                t[r.randrange(256)] = r.randrange(-256, 384)
        for t in (crg, cbg):
            for _ in range(r.randrange(1, 20)):
                t[r.randrange(256)] = r.randrange(-128 << 16, 128 << 16)
    return struct.pack('<256i', *crr) + struct.pack('<256i', *cbb) + struct.pack('<256i', *crg) + struct.pack('<256i', *cbg)


def converting(r, components):
    """cinfo, cconvert, the tables, the range limit, an input image of components (rows input_row.. of each) and output rows."""
    width = r.choice([0, 1, 2, 7, 8, r.randrange(1, 100)])
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, OUTPUT_WIDTH, width)
    struct.pack_into('<i', cinfo, NUM_COMPONENTS, components)
    struct.pack_into('<I', cinfo, SAMPLE_RANGE_LIMIT, RANGE + 256)
    struct.pack_into('<I', cinfo, CCONVERT, CC)
    cc = bytearray(r.bytes(0x18))
    struct.pack_into('<4I', cc, 8, TABLES, TABLES + 0x400, TABLES + 0x800, TABLES + 0xc00)
    input_row, num_rows = r.randrange(4), r.choice([0, 1, 1, 2, r.randrange(1, 8)])
    rows = input_row + num_rows
    stride = width + r.randrange(4)
    comp_rows = b''.join(struct.pack('<I', IN + (c * rows + i) * stride) for c in range(components) for i in range(rows))
    out_stride = width * max(components, 3) + r.randrange(4)
    regions = {CINFO: cinfo, CC: cc, TABLES: tables(r), RANGE: r.bytes(RANGE_SIZE),
               IMAGE: struct.pack('<%dI' % components, *(COMP_ROWS + c * rows * 4 for c in range(components))), COMP_ROWS: comp_rows,
               IN: r.bytes(components * rows * stride + 1), OUT_ROWS: struct.pack('<%dI' % num_rows, *(OUT + i * out_stride for i in range(num_rows))),
               OUT: r.bytes(num_rows * out_stride + 1)}
    for k, v in regions.items():  # (none empty: the driver reads a region's hex as a whitespace-delimited token)
        if not v:
            regions[k] = r.bytes(4)
    return [CINFO, IMAGE, input_row, OUT_ROWS, num_rows], regions


def converter(address, components):
    def op(r):
        args, regions = converting(r, components if components else r.randrange(1, 5))
        if r.random() < 0.03:  # (num_rows below zero: nothing)
            args[4] = -r.randrange(1, 3) & 0xffffffff
        return Call(address, args, regions)
    op.__name__ = {0x646b00: 'ycc_rgb', 0x646c40: 'null', 0x646cd0: 'grayscale', 0x646d00: 'gray_rgb'}[address]
    return op


def build(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<I', cinfo, MEM_, MEM)
    struct.pack_into('<I', cinfo, CCONVERT, CC)
    return Call(0x646a30, [CINFO], {CINFO: cinfo, MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), CC: r.bytes(0x18), HEAP: heap(0x1100)})


OPS = [f for f in (build, converter(0x646b00, 3), converter(0x646c40, 0), converter(0x646cd0, 1), converter(0x646d00, 1))
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
