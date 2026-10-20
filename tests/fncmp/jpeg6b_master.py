# libjpeg 6b's jdmaster.c (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b, the module through its wrapper unit
# third_party/wrap/jpeg-6b/jdmaster.c) against the exe's copies. cinfo (0x1b0) with its components at COMPS; alloc_small is fncmp's
# ZALLOC stub over the case heap (JPOOL_IMAGE 1 is ZALLOC's items), alloc_large, alloc_sarray / barray and request_virt_sarray /
# barray are cdecl recorders answering BIG, ROWS (eight rows of 0x300 samples) and VIRTUAL, realize_virt_arrays and the input
# controller's start_input_pass are recorders, as are the other modules' methods prepare_for_output_pass and finish_output_pass call
# (the quantizers' start_pass take a boolean: logged as its low byte, as are request_virt_*'s pre_zero).
# master_selection (and jinit_master_decompress before it) runs the other modules' inits, which are already the library's: the Bench's
# ranges are their exe copies (and what they call), the driver runs the library's through the method shims, so a case compares the
# whole selection. Deal only what those inits can meet: 1..4 components (jquant1's select_ncolors reads Ncolors[] past four), sampling
# factors 1..4 with the maxima set as jdinput.c sets them (a factor of 0 divides by zero in jdsample.c), and mostly at least 2 ^ components
# colours (fewer is JERR_QUANT_FEW_COLORS, past which select_ncolors leaves a component one colour and output_value divides by zero).
# usage: jpeg6b_master.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=merged,calc,range,select,jinit,prepare,finish)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, LOG_BYTES, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); then what master_selection runs (the other modules' inits, jutils)
FUNCS = {'jpeg_calc_output_dimensions': (0x63e0b0, 0x63e2d0), 'use_merged_upsample': (0x63e2f0, 0x63e380),
         'jinit_master_decompress': (0x63e380, 0x63e3c0), 'master_selection': (0x63e3c0, 0x63e5b0),
         'prepare_range_limit_table': (0x63e5b0, 0x63e630), 'prepare_for_output_pass': (0x63e630, 0x63e790),
         'finish_output_pass': (0x63e790, 0x63e7c0)}
INITS = [(0x6428b0, 0x6428e0), (0x642b30, 0x642cc0), (0x643240, 0x643350), (0x6446d0, 0x644710), (0x644d30, 0x644db0),
         (0x645a80, 0x645b00), (0x645d50, 0x645de0), (0x6460c0, 0x6462e0), (0x646860, 0x646b00), (0x646ee0, 0x647040),
         (0x6474f0, 0x6476c0), (0x6488f0, 0x648db0)]
K_ERROR, K_EMIT, K_CQ_START, K_CQ_FINISH, K_POST, K_MAIN, K_IDCT, K_COEF, K_CCONV, K_UPS = range(10)
K_LARGE, K_SARRAY, K_BARRAY, K_VSARRAY, K_VBARRAY, K_REALIZE, K_START_INPUT = range(10, 17)
K_Q1_START, K_Q1_FINISH, K_Q2_START, K_Q2_FINISH, K_PROGRESS = range(17, 22)
B = Bench(sorted(FUNCS.values()) + INITS)

CINFO, COMPS, ERR, MEM, INPUTCTL, PROGRESS, MASTER, MODS = (RAM, RAM + 0x200, RAM + 0x400, RAM + 0x500, RAM + 0x580, RAM + 0x5a0,
                                                           RAM + 0x5c0, RAM + 0x600)
ROWS, SAMPLES, BIG, VIRTUAL = RAM + 0x800, RAM + 0x1000, RAM + 0x2800, RAM + 0x2c00
CQ, Q1, Q2, POST, MAIN, IDCT, COEF, CCONV, UPS = (MODS + 0x10 * i for i in range(9))
SPACES = [(1, 1, 1), (3, 2, 3), (3, 2, 3), (3, 3, 3), (3, 1, 3), (2, 2, 3), (2, 1, 3), (4, 4, 4), (5, 4, 4), (0, 0, 2), (1, 2, 1)]


def common(r):
    log = log_region()
    set_return(log, K_LARGE, BIG)
    set_return(log, K_SARRAY, ROWS)
    set_return(log, K_BARRAY, ROWS)
    set_return(log, K_VSARRAY, VIRTUAL)
    set_return(log, K_VBARRAY, VIRTUAL)
    for k, mask in ((K_CQ_START, 2), (K_Q1_START, 2), (K_Q2_START, 2), (K_VSARRAY, 4), (K_VBARRAY, 4)):
        struct.pack_into('<I', log, LOG_BYTES + 4 * k, mask)
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<III', cinfo, 0, ERR, MEM, PROGRESS if r.random() < 0.5 else 0)
    struct.pack_into('<I', cinfo, 0xc4, COMPS)
    struct.pack_into('<I', cinfo, 0x180, MASTER)
    struct.pack_into('<II', cinfo, 0x184, MAIN, COEF)
    struct.pack_into('<II', cinfo, 0x18c, POST, INPUTCTL)
    struct.pack_into('<I', cinfo, 0x19c, IDCT)
    struct.pack_into('<III', cinfo, 0x1a0, UPS, CCONV, CQ)
    err = struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x7c)
    mem = struct.pack('<7I', ZALLOC, recorder(K_LARGE, 3, cdecl=True), recorder(K_SARRAY, 4, cdecl=True), recorder(K_BARRAY, 4, cdecl=True),
                      recorder(K_VSARRAY, 6, cdecl=True), recorder(K_VBARRAY, 6, cdecl=True), recorder(K_REALIZE, 1, cdecl=True)) + r.bytes(0x18)
    inputctl = struct.pack('<4I', 0, 0, recorder(K_START_INPUT, 1, cdecl=True), 0) + bytes([r.choice([0, 1]), r.choice([0, 1])]) + r.bytes(2)
    progress = struct.pack('<I', recorder(K_PROGRESS, 1, cdecl=True)) + r.bytes(0x10)
    master = bytearray(r.bytes(0x1c))
    master[8], master[0x10] = r.choice([0, 0, 1]), r.choice([0, 1])
    struct.pack_into('<II', master, 0x14, Q1, Q2)
    mods = bytearray(r.bytes(0x90))
    for at, start, finish in ((CQ, K_CQ_START, K_CQ_FINISH), (Q1, K_Q1_START, K_Q1_FINISH), (Q2, K_Q2_START, K_Q2_FINISH)):
        struct.pack_into('<I', mods, at - MODS, recorder(start, 2, cdecl=True))
        struct.pack_into('<I', mods, at - MODS + 8, recorder(finish, 1, cdecl=True))
    for at, k, n in ((POST, K_POST, 2), (MAIN, K_MAIN, 2), (IDCT, K_IDCT, 1), (CCONV, K_CCONV, 1), (UPS, K_UPS, 1)):
        struct.pack_into('<I', mods, at - MODS, recorder(k, n, cdecl=True))
    struct.pack_into('<I', mods, COEF - MODS + 8, recorder(K_COEF, 1, cdecl=True))
    regs = {CINFO: cinfo, ERR: err, MEM: mem, INPUTCTL: inputctl, PROGRESS: progress, MASTER: master, MODS: mods, LOG: log,
            HEAP: heap(0x3000), ROWS: struct.pack('<8I', *[SAMPLES + 0x300 * i for i in range(8)]), SAMPLES: r.bytes(0x1800),
            BIG: r.bytes(0x400), VIRTUAL: r.bytes(0x10)}
    return regs


def image(r, regs, merged=False):
    """The fields the output dimensions and the inits read: sizes, colour spaces, components, scale, the quantizer's options."""
    cinfo = regs[CINFO]
    if merged:
        jcs, ocs, nc = 3, 2, 3
    elif r.random() < 0.9:
        jcs, ocs, nc = r.choice(SPACES)
    else:
        jcs, ocs, nc = r.randrange(6), r.randrange(6), r.choice([1, 2, 3, 4])
    comps = bytearray(r.bytes(0x54 * 4))
    if merged:
        hv = [(2, r.choice([1, 2])), (1, 1), (1, 1)]
    else:
        f = [1, 1, 1, 2, 2, 4, 3] if r.random() < 0.2 else [1, 1, 2]
        hv = [(r.choice(f), r.choice(f)) for _ in range(nc)]
    for ci, (h, v) in enumerate(hv):
        struct.pack_into('<4i', comps, 0x54 * ci, ci + 1, ci, h, v)
    regs[COMPS] = comps
    struct.pack_into('<i', cinfo, 0x14, 0xca if r.random() < 0.97 else r.choice([0xc9, 0xcd]))
    struct.pack_into('<IIiii', cinfo, 0x1c, r.choice([1, 7, 8, 16, 17, r.randrange(1, 101)]), r.choice([1, 8, 9, r.randrange(1, 101)]), nc, jcs, ocs)
    struct.pack_into('<II', cinfo, 0x30, *r.choice([(1, 1), (1, 1), (1, 2), (1, 4), (1, 8), (3, 8), (1, 16), (2, 1), (r.randrange(1, 9), 8)]))
    struct.pack_into('<ii', cinfo, 0x110, max(h for h, v in hv), max(v for h, v in hv))
    cinfo[0x48] = r.choice([0, 1]) if not merged or r.random() < 0.2 else 0
    cinfo[0x10a] = 0 if r.random() < 0.9 else 1
    cinfo[0x40], cinfo[0x41], cinfo[0x4a] = r.choice([0, 0, 1]), 0 if r.random() < 0.9 else 1, r.choice([0, 0, 1])
    struct.pack_into('<i', cinfo, 0x4c, r.choice([0, 1, 2]))
    cinfo[0x50] = r.choice([0, 1])
    colors = r.choice([8, 16, 27, 64, 256, r.randrange(8, 257)])
    struct.pack_into('<i', cinfo, 0x54, max(colors, 1 << nc) if r.random() < 0.97 else r.choice([2, 4, 300]))
    cinfo[0x58], cinfo[0x59], cinfo[0x5a] = r.choice([0, 1]), r.choice([0, 1]), r.choice([0, 1])
    struct.pack_into('<I', cinfo, 0x74, 0 if r.random() < 0.7 else ROWS)
    cinfo[0xc8], cinfo[0xc9] = r.choice([0, 1]), 0 if r.random() < 0.95 else 1
    struct.pack_into('<I', cinfo, 0x11c, r.randrange(1, 20))


def merged(r):
    regs = common(r)
    image(r, regs, r.random() < 0.5)
    cinfo = regs[CINFO]
    if r.random() < 0.5:  # (the DCT sizes as jpeg_calc_output_dimensions leaves them, or one off)
        m = r.choice([1, 2, 4, 8])
        struct.pack_into('<i', cinfo, 0x118, m)
        for ci in range(3):
            struct.pack_into('<i', regs[COMPS], 0x54 * ci + 0x24, m if r.random() < 0.85 else m * 2)
    if r.random() < 0.1:
        struct.pack_into('<i', cinfo, 0x64, r.choice([1, 4]))
    else:
        struct.pack_into('<i', cinfo, 0x64, 3)
    return Call(0x63e2f0, [CINFO], regs, ret='b')


def calc(r):
    regs = common(r)
    image(r, regs, r.random() < 0.3)
    return Call(0x63e0b0, [CINFO], regs)


def range_(r):
    return Call(0x63e5b0, [CINFO], common(r))


def select(r):
    regs = common(r)
    image(r, regs, r.random() < 0.3)
    return Call(0x63e3c0, [CINFO], regs)


def jinit(r):
    regs = common(r)
    image(r, regs, r.random() < 0.3)
    return Call(0x63e380, [CINFO], regs)


def prepare(r):
    regs = common(r)
    cinfo = regs[CINFO]
    cinfo[0x4a], cinfo[0x50], cinfo[0x58], cinfo[0x5a] = r.choice([0, 1, 1]), r.choice([0, 1]), r.choice([0, 1]), r.choice([0, 1])
    cinfo[0x40], cinfo[0x41] = r.choice([0, 1]), 0 if r.random() < 0.8 else 1
    struct.pack_into('<I', cinfo, 0x74, 0 if r.random() < 0.7 else ROWS)
    return Call(0x63e630, [CINFO], regs)


def finish(r):
    regs = common(r)
    regs[CINFO][0x4a] = r.choice([0, 1])
    return Call(0x63e790, [CINFO], regs)


OPS = [f for f in (merged, calc, range_, select, jinit, prepare, finish)
       if not os.environ.get('OPS') or f.__name__.rstrip('_') in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
