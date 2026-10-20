# libjpeg 6a's jccolor.c, the compressor's input colour converter (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a,
# the module through its wrapper unit third_party/wrap/jpeg-6a/jccolor.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit a cdecl recorder that returns; mem +4 at MEM, alloc_small fncmp's ZALLOC over the
# case heap; image_width +0x18, input_components +0x20, in_color_space +0x24, num_components +0x34, jpeg_color_space +0x38;
# cconvert +0x158 at CC (my_color_converter, 0xc bytes: start_pass, color_convert, rgb_ycc_tab +8 at TAB, 8 * 256 INT32s)).
# jinit_color_converter's ERREXITs return, so its cases meet every colour space pair, valid or not (J_COLOR_SPACE 0..5 and one
# past). The converters' rows are real: input_buf's row pointers into IROWS (rows of up to 32 pixels of 4 samples), output_buf's
# four components' row pointers into OROWS (eight rows of 32 samples each), output_row + num_rows at most eight; the table is random
# (the converters only index it by sample values, 0..255 each part). grayscale_convert's stride (input_components) and null_convert's
# num_components stay 1..4.
# usage: jpeg6a_color.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,ycc,gray,cmyk,grayscale,null)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_color_converter': (0x6554d0, 0x65586f), 'rgb_ycc_start': (0x65586f, 0x655980),
         'rgb_ycc_convert': (0x655980, 0x655ae4), 'rgb_gray_convert': (0x655ae4, 0x655bbf),
         'cmyk_ycck_convert': (0x655bbf, 0x655d55), 'grayscale_convert': (0x655d55, 0x655ddc),
         'null_convert': (0x655ddc, 0x655e8d)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, CC, IPTRS, OBUF, IROWS, OROWS, TAB = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x340, RAM + 0x400, RAM + 0x800, RAM + 0x2000)
K_ERROR = 0
W = 32  # (the widest row, in pixels)


def common(r, ncomps=None, incomps=None):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    space = r.randrange(7)
    if incomps is None:
        incomps = r.choice([1, 3, 4, [0, 1, 1, 3, 3, 4][min(space, 5)], r.randrange(6)])
    if ncomps is None:
        ncomps = r.choice([1, 3, 4, r.randrange(1, 5)])
    struct.pack_into('<IIi', cinfo, 0x18, r.randrange(1, W + 1), r.getrandbits(32), incomps)
    struct.pack_into('<i', cinfo, 0x24, space)
    struct.pack_into('<ii', cinfo, 0x34, ncomps, r.choice([space, r.randrange(7)]))
    struct.pack_into('<I', cinfo, 0x158, CC)
    cc = bytearray(r.bytes(0xc))
    struct.pack_into('<I', cc, 8, TAB)
    iptrs = b''.join(struct.pack('<I', IROWS + 4 * W * j) for j in range(8))
    obuf = struct.pack('<4I', *[OBUF + 0x10 + 0x20 * ci for ci in range(4)]) + \
        b''.join(struct.pack('<I', OROWS + W * (8 * ci + j)) for ci in range(4) for j in range(8))
    return {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c), MEM: struct.pack('<I', ZALLOC) +
            r.bytes(0x2c), CC: cc, LOG: log_region(), IPTRS: iptrs, OBUF: obuf, IROWS: r.bytes(4 * W * 8), OROWS: r.bytes(W * 32)}


def jinit(r):
    regs = common(r)
    if r.random() < 0.5:  # (a pair the library converts)
        space, jspace = r.choice([(1, 1), (2, 1), (3, 1), (2, 2), (2, 3), (3, 3), (4, 4), (4, 5), (5, 5), (0, 0), (6, 6)])
        nc = {1: 1, 2: 3, 3: 3, 4: 4, 5: 4}.get(jspace, r.randrange(1, 5))
        struct.pack_into('<i', regs[CINFO], 0x20, nc if space else r.choice([nc, r.randrange(1, 5)]))
        struct.pack_into('<i', regs[CINFO], 0x24, space)
        struct.pack_into('<ii', regs[CINFO], 0x34, nc, jspace)
    regs[HEAP] = heap(0x100)
    return Call(0x6554d0, [CINFO], regs)


def start(r):
    regs = common(r)
    regs[HEAP] = heap(0x2100)
    return Call(0x65586f, [CINFO], regs)


def convert(r, address, ncomps=None, incomps=None):
    regs = common(r, ncomps, incomps)
    regs[TAB] = r.bytes(0x2000)
    row = r.choice([0, 0, r.randrange(8)])
    return Call(address, [CINFO, IPTRS, OBUF, row, r.choice([0, 1, 2, r.randrange(-1, 8 - row + 1)])], regs)


def ycc(r):
    return convert(r, 0x655980)


def gray(r):
    return convert(r, 0x655ae4)


def cmyk(r):
    return convert(r, 0x655bbf)


def grayscale(r):
    return convert(r, 0x655d55, incomps=r.randrange(1, 5))


def null(r):
    return convert(r, 0x655ddc, ncomps=r.randrange(1, 5))


OPS = [f for f in (jinit, start, ycc, gray, cmyk, grayscale, null)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
