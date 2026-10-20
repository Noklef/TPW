# libjpeg 6a's jcdctmgr.c, the compressor's forward DCT manager (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a,
# the module through its wrapper unit third_party/wrap/jpeg-6a/jcdctmgr.c, compiled /arch:IA32) against the exe's 6a copies. cinfo
# at CINFO (6a's jpeg_compress_struct: err +0 at ERR, error_exit a cdecl recorder that returns; mem +4 at MEM, alloc_small fncmp's
# ZALLOC over the case heap; num_components +0x34, comp_info +0x3c at COMPS (quant_tbl_no +0x10), quant_tbl_ptrs[4] +0x40 into QT
# (quantval[64], u16), dct_method +0xbc; fdct +0x160 at FD (my_fdct_controller, 0x30 bytes: start_pass, forward_DCT, do_dct +8,
# divisors[4] +0xc, do_float_dct +0x1c, float_divisors[4] +0x20)). forward_DCT_float stays the exe's (see jpeg6a.h).
# start_pass_fdctmgr's cases keep quant_tbl_no in 0..3 (outside it the source, its ERREXIT returning, reads quant_tbl_ptrs[-1] and
# writes a table through fdct's do_dct or do_float_dct) and leave a table pointer null in one case in twenty (a read of null in
# both); its JDCT_FLOAT divisors are x87 code in the exe, run at the game's single precision (cw 0x7f) and at double (0x27f).
# forward_DCT's do_dct is a cdecl recorder logging the workspace (on the stack) as 0, and filling its first 28 DCTELEMs with
# values up to 2^20 one case in two (the samples alone give -128..127); the samples are real rows (SROWS, 16 rows of 40), the
# divisors positive (DIV), the coefficients go to BLOCKS.
# usage: jpeg6a_dctmgr.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,dct)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fill, heap, log_region, recorder, strings  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_forward_dct': (0x653610, 0x6536f7), 'start_pass_fdctmgr': (0x6536f7, 0x65398f),
         'forward_DCT': (0x65398f, 0x653d32)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, FD, COMPS, QT, DIV, FDIV, SPTRS, SROWS, BLOCKS = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x680, RAM + 0x900, RAM + 0xd00, RAM + 0x1100, RAM + 0x1180,
    RAM + 0x1500)
K_ERROR, K_DCT, K_FLOAT = range(3)
W = 40  # (a sample row)


def common(r):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    ncomps = r.choice([1, 3, 3, 4, 2])
    struct.pack_into('<i4xI', cinfo, 0x34, ncomps, COMPS)
    for q in range(4):
        struct.pack_into('<I', cinfo, 0x40 + 4 * q, QT + 0x84 * q)
    struct.pack_into('<i', cinfo, 0xbc, r.choice([0, 1, 2, 0, 1, 2, 3, -1]))
    struct.pack_into('<I', cinfo, 0x160, FD)
    comps = bytearray(r.bytes(0x54 * 10))
    for c in range(10):
        struct.pack_into('<i', comps, 0x54 * c + 0x10, r.choice([0, 0, 1, 1, 2, 3]))
    qt = bytearray(r.bytes(0x84 * 4))
    for q in range(4):
        struct.pack_into('<64H', qt, 0x84 * q, *[r.choice([1, r.randrange(1, 256), r.randrange(1, 256), r.randrange(0x8000), 0])
                                                for _ in range(64)])
    fd = bytearray(r.bytes(0x30))
    struct.pack_into('<I', fd, 8, recorder(K_DCT, 1, cdecl=True))
    struct.pack_into('<I', fd, 0x1c, recorder(K_FLOAT, 1, cdecl=True))
    for q in range(4):
        struct.pack_into('<I', fd, 0xc + 4 * q, 0 if r.random() < 0.4 else DIV + 0x100 * q)
        struct.pack_into('<I', fd, 0x20 + 4 * q, 0 if r.random() < 0.4 else FDIV + 0x100 * q)
    div = b''.join(struct.pack('<i', r.choice([8, r.randrange(1, 0x800), r.randrange(1, 0x40000)])) for _ in range(0x100))
    log = log_region()
    strings(log, K_DCT, 0, out=1)
    if r.random() < 0.5:
        fill(log, K_DCT, 0, b''.join(struct.pack('<i', r.randrange(-1 << 20, 1 << 20)) for _ in range(28)))
    sptrs = b''.join(struct.pack('<I', SROWS + W * j) for j in range(16))
    return {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c),
            MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), FD: fd, COMPS: comps, QT: qt, DIV: div, FDIV: r.bytes(0x400), LOG: log,
            SPTRS: sptrs, SROWS: r.bytes(W * 16), BLOCKS: r.bytes(0x80 * 4)}


def jinit(r):
    regs = common(r)
    regs[HEAP] = heap(0x100)
    return Call(0x653610, [CINFO], regs)


def start(r):
    regs = common(r)
    if r.random() < 0.05:
        struct.pack_into('<I', regs[CINFO], 0x40 + 4 * r.randrange(4), 0)
    regs[HEAP] = heap(0x800)
    return Call(0x6536f7, [CINFO], regs, cw=r.choice([0x7f, 0x27f]))


def dct(r):
    regs = common(r)
    for q in range(4):
        struct.pack_into('<I', regs[FD], 0xc + 4 * q, DIV + 0x100 * q)
    nb = r.choice([1, 1, 2, 3, 4])
    col = r.randrange(W - 8 * nb + 1)
    return Call(0x65398f, [CINFO, COMPS + 0x54 * r.randrange(4), SPTRS, BLOCKS, r.randrange(9), col, nb], regs)


OPS = [f for f in (jinit, start, dct) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
