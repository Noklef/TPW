# libjpeg 6a's jcsample.c, the compressor's downsampler (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, the module
# through its wrapper unit third_party/wrap/jpeg-6a/jcsample.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit +0 and emit_message +4 cdecl recorders that return; mem +4 at MEM, alloc_small
# fncmp's ZALLOC over the case heap; image_width +0x18, num_components +0x34, comp_info +0x3c at COMPS (h / v_samp_factor +8 / +0xc,
# width_in_blocks +0x1c), CCIR601_sampling +0xb4, smoothing_factor +0xb8, max_h / v_samp_factor +0xe0 / +0xe4; downsample +0x15c at
# DS (my_downsampler, 0x34 bytes: start_pass, downsample, need_context_rows +8, methods[10] +0xc, cdecl recorders for
# sep_downsample)). The exe's build has INPUT_SMOOTHING_SUPPORTED. start_pass_downsample is empty and stays the exe's (a hand port).
# jinit_downsampler's ERREXITs return, so its cases meet every sampling pair, supported or not. The single-component methods get the
# component's factors their selection in jinit_downsampler implies (fullsize: the maxima; h2v1: half the horizontal; h2v2: half
# both; int: divisors of the maxima), widths of 1..2 blocks and real rows: input_data points at the second of nine row pointers into
# IROWS (the smoothing methods read the row above and max_v_samp_factor + 1 below), output_data at eight into OROWS, rows of 64
# samples (16 output columns at a horizontal ratio of up to 4), image_width up to the expanded width.
# usage: jpeg6a_sample.py [CASES [SEED]] [--wild|--both] [--show N]
#        (env OPS=jinit,sep,expand,int,full,h2v1,h2v2,h2v2s,fulls)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); then jutils.c's jcopy_sample_rows
FUNCS = {'jinit_downsampler': (0x6547f0, 0x654a1d), 'sep_downsample': (0x654a22, 0x654ab1),
         'int_downsample': (0x654ab1, 0x654c1d), 'expand_right_edge': (0x654c1d, 0x654c94),
         'fullsize_downsample': (0x654c94, 0x654ce5), 'h2v1_downsample': (0x654ce5, 0x654db3),
         'h2v2_downsample': (0x654db3, 0x654ebf), 'h2v2_smooth_downsample': (0x654ebf, 0x65527e),
         'fullsize_smooth_downsample': (0x65527e, 0x6554cb)}
CALLED = [(0x650401, 0x65047f)]
B = Bench(sorted(FUNCS.values()) + CALLED)

CINFO, ERR, MEM, DS, COMPS, IPTRS, OPTRS, IROWS, OROWS = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x680, RAM + 0x6c0, RAM + 0x800, RAM + 0xc00)
K_ERROR, K_EMIT, K_METHOD = 0, 1, 2
W = 64  # (a row's samples)


def common(r, ncomps=None):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    if ncomps is None:
        ncomps = r.choice([1, 3, 3, 4, 2])
    struct.pack_into('<i4xI', cinfo, 0x34, ncomps, COMPS)
    struct.pack_into('<ii', cinfo, 0xb4, 0 if r.random() < 0.85 else r.choice([1, 0x100]),
                     r.choice([0, 0, r.randrange(1, 101), 100]))
    mh, mv = r.choice([1, 2, 2, 3, 4]), r.choice([1, 2, 2, 3, 4])
    struct.pack_into('<ii', cinfo, 0xe0, mh, mv)
    struct.pack_into('<I', cinfo, 0x15c, DS)
    comps = bytearray(r.bytes(0x54 * 10))
    for c in range(10):
        h = r.choice([mh, mh, max(1, mh // 2), r.randrange(1, 5)])
        v = r.choice([mv, mv, max(1, mv // 2), r.randrange(1, 5)])
        struct.pack_into('<ii', comps, 0x54 * c + 8, h, v)
        struct.pack_into('<I', comps, 0x54 * c + 0x1c, r.randrange(1, 3))
    ds = bytearray(r.bytes(0x34))
    for ci in range(10):
        struct.pack_into('<I', ds, 0xc + 4 * ci, recorder(K_METHOD, 4, cdecl=True))
    err = bytearray(r.bytes(0x80))
    struct.pack_into('<II', err, 0, recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True))
    iptrs = b''.join(struct.pack('<I', IROWS + W * j) for j in range(9))
    optrs = b''.join(struct.pack('<I', OROWS + W * j) for j in range(8))
    width = W
    struct.pack_into('<I', cinfo, 0x18, r.choice([1, 8, 16, r.randrange(1, width + 1)]))
    return {CINFO: cinfo, ERR: err, MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), DS: ds, COMPS: comps, LOG: log_region(),
            IPTRS: iptrs, OPTRS: optrs, IROWS: r.bytes(W * 9), OROWS: r.bytes(W * 8)}


def jinit(r):
    regs = common(r, r.choice([1, 3, 3, 4, 2, 10]))
    regs[HEAP] = heap(0x100)
    return Call(0x6547f0, [CINFO], regs)


def sep(r):
    regs = common(r)
    return Call(0x654a22, [CINFO, IPTRS, r.choice([0, 1, 2, r.randrange(8)]), OPTRS,
                           r.choice([0, 1, 2, r.randrange(8)])], regs)


def expand(r):
    regs = common(r)
    return Call(0x654c1d, [IPTRS, r.randrange(10), r.randrange(1, W + 1), r.choice([0, W, r.randrange(1, W + 1)])], regs)


def one(r, address, hdiv, vdiv, smooth=False):
    """A component of the maxima over hdiv / vdiv (None: any divisor) for the method at address."""
    regs = common(r)
    cinfo, comps = regs[CINFO], regs[COMPS]
    mh, mv = struct.unpack_from('<ii', cinfo, 0xe0)
    if hdiv == 2 and mh % 2:
        mh += 1
    if vdiv == 2 and mv % 2:
        mv += 1
    mh, mv = min(mh, 4), min(mv, 4)
    struct.pack_into('<ii', cinfo, 0xe0, mh, mv)
    h = mh // hdiv if hdiv else r.choice([d for d in (1, 2, 3, 4) if mh % d == 0])
    v = mv // vdiv if vdiv else r.choice([d for d in (1, 2, 3, 4) if mv % d == 0])
    struct.pack_into('<ii', comps, 8, h, v)
    wib = struct.unpack_from('<I', comps, 0x1c)[0]
    struct.pack_into('<I', cinfo, 0x18, r.choice([1, wib * 8 * (mh // h), r.randrange(1, wib * 8 * (mh // h) + 1)]))
    if not smooth:
        struct.pack_into('<i', cinfo, 0xb8, 0)
    return Call(address, [CINFO, COMPS, IPTRS + 4, OPTRS], regs)


def int_(r):
    return one(r, 0x654ab1, None, None)


def full(r):
    return one(r, 0x654c94, 1, 1)


def h2v1(r):
    return one(r, 0x654ce5, 2, 1)


def h2v2(r):
    return one(r, 0x654db3, 2, 2)


def h2v2s(r):
    return one(r, 0x654ebf, 2, 2, smooth=True)


def fulls(r):
    return one(r, 0x65527e, 1, 1, smooth=True)


int_.__name__ = 'int'
OPS = [f for f in (jinit, sep, expand, int_, full, h2v1, h2v2, h2v2s, fulls)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
