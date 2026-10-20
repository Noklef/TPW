# libjpeg 6a's jcprepct.c, the compressor's preprocessing controller (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a,
# the module through its wrapper unit third_party/wrap/jpeg-6a/jcprepct.c) against the exe's 6a copies. cinfo at CINFO (6a's
# jpeg_compress_struct: err +0 at ERR, error_exit a cdecl recorder; mem +4 at MEM, alloc_small fncmp's ZALLOC over the case heap and
# alloc_sarray +8 a recorder answering TRUE; image_width / height +0x18 / +0x1c, num_components +0x34, comp_info +0x3c at COMPS
# (h / v_samp_factor +8 / +0xc, width_in_blocks +0x1c), max_h / v_samp_factor +0xe0 / +0xe4; prep +0x14c at PREP (my_prep_controller,
# 0x40 bytes: start_pass, pre_process_data, color_buf[10] +8, rows_to_go +0x30, next_buf_row +0x34, this_row_group +0x38,
# next_buf_stop +0x3c); cconvert +0x158 (color_convert +4) and downsample +0x15c (downsample +4, need_context_rows +8) in MODS, the
# methods cdecl recorders). The exe's build has CONTEXT_ROWS_SUPPORTED (pre_process_context and create_context_buffer are there).
# Rows are real, for jcopy_sample_rows (the exe's jutils.c copy on its side): each component's color_buf is the middle of five row
# groups of row pointers into CROWS (as create_context_buffer lays them out, so the context case's negative rows are there too), and
# output_buf's rows are in OROWS, one more before each component's (the source never reads it); widths at most 64 samples. The
# states stay where the source does: next_buf_row below max_v_samp_factor for pre_process_data, next_buf_row <= next_buf_stop <=
# 3 * max_v_samp_factor and this_row_group a row group's start for pre_process_context.
# usage: jpeg6a_prepct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,data,expand,context,create)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); then jutils.c's jcopy_sample_rows
FUNCS = {'jinit_c_prep_controller': (0x654090, 0x654188), 'start_pass_prep': (0x654188, 0x6541ed),
         'pre_process_data': (0x6541ed, 0x6543fc), 'expand_bottom_edge': (0x6543fc, 0x654440),
         'pre_process_context': (0x654440, 0x6546a5), 'create_context_buffer': (0x6546a5, 0x6547f0)}
CALLED = [(0x650401, 0x65047f)]
B = Bench(sorted(FUNCS.values()) + CALLED)

CINFO, ERR, MEM, PREP, MODS, COMPS, CTR, TRUE, INPUT, CPTRS, OPTRS, CROWS, OROWS = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x300, RAM + 0x340, RAM + 0x700, RAM + 0x710, RAM + 0x780, RAM + 0x800,
    RAM + 0xa00, RAM + 0x1000, RAM + 0x3000)
K_ERROR, K_SARRAY, K_CONVERT, K_DOWN = range(4)
W = 64  # (the widest row)


def crow(ci, j):  # component ci's j-th row pointer of five groups of four
    return CROWS + (ci * 20 + j) * W


def common(r, ncomps=None, vmax=None):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    if ncomps is None:
        ncomps = r.choice([1, 3, 3, 4, 2])
    if vmax is None:
        vmax = r.choice([1, 2, 2, 4])
    height = r.choice([1, 8, 16, r.randrange(1, 40)])
    struct.pack_into('<II', cinfo, 0x18, r.randrange(1, W + 1), height)
    struct.pack_into('<i4xI', cinfo, 0x34, ncomps, COMPS)
    struct.pack_into('<ii', cinfo, 0xe0, r.choice([1, 2, 4]), vmax)
    struct.pack_into('<I', cinfo, 0x14c, PREP)
    struct.pack_into('<II', cinfo, 0x158, MODS, MODS + 0x10)
    comps = bytearray(r.bytes(0x54 * 10))
    for c in range(10):
        struct.pack_into('<ii', comps, 0x54 * c + 8, r.choice([1, 2, 4]), r.choice([v for v in (1, 2, 4) if v <= vmax]))
        struct.pack_into('<I', comps, 0x54 * c + 0x1c, r.randrange(1, W // 8 + 1))
    mods = bytearray(r.bytes(0x20))
    struct.pack_into('<I', mods, 4, recorder(K_CONVERT, 5, cdecl=True))
    struct.pack_into('<II', mods, 0x14, recorder(K_DOWN, 5, cdecl=True), r.choice([0, 0, 1, 0x100]))
    log = log_region()
    set_return(log, K_SARRAY, TRUE)
    prep = bytearray(r.bytes(0x40))
    for ci in range(10):
        struct.pack_into('<I', prep, 8 + 4 * ci, CPTRS + 0x50 * min(ci, 3) + 4 * vmax)
    cptrs = b''.join(struct.pack('<I', crow(ci, j)) for ci in range(4) for j in range(20))
    optrs = struct.pack('<4I', *[OPTRS + 0x20 + 0x84 * ci + 4 for ci in range(4)]) + bytes(0x10) + \
        b''.join(struct.pack('<I', OROWS + (ci * 33 + k) * W) for ci in range(4) for k in range(33))
    return {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c),
            MEM: struct.pack('<III', ZALLOC, r.getrandbits(32), recorder(K_SARRAY, 4, cdecl=True)) + r.bytes(0x24),
            PREP: prep, MODS: mods, COMPS: comps, LOG: log, TRUE: r.bytes(0x40), CPTRS: cptrs, OPTRS: optrs,
            CROWS: r.bytes(4 * 20 * W), OROWS: r.bytes(4 * 33 * W), INPUT: r.bytes(0x40)}


def jinit(r):
    regs = common(r, r.choice([1, 3, 3, 4, 2, 10, 0]))
    regs[HEAP] = heap(0x1000)
    return Call(0x654090, [CINFO, r.choice([0, 0, 0, 1])], regs)


def start(r):
    return Call(0x654188, [CINFO, r.choice([0, 0, 0, 1, 2, 3])], common(r))


def counters(r, regs, avail):
    in_avail = r.choice([0, 1, 2, 8, 16, r.randrange(40)])
    regs[CTR] = struct.pack('<II', r.choice([0, 0, in_avail, r.randrange(in_avail + 1)]), r.choice([0, 0, 1, r.randrange(avail + 1)]))
    return in_avail


def data(r):
    regs = common(r)
    vmax = struct.unpack_from('<i', regs[CINFO], 0xe4)[0]
    height = struct.unpack_from('<I', regs[CINFO], 0x1c)[0]
    struct.pack_into('<Ii', regs[PREP], 0x30, r.choice([0, 1, 2, vmax, height, r.randrange(20)]), r.randrange(vmax))
    avail = r.choice([8, 8, 1, 2])
    in_avail = counters(r, regs, avail)
    return Call(0x6541ed, [CINFO, INPUT, CTR, in_avail, OPTRS, CTR + 4, avail], regs)


def expand(r):
    regs = common(r, vmax=4)
    return Call(0x6543fc, [CPTRS + 0x10, r.randrange(W + 1), r.randrange(17), r.choice([0, 16, r.randrange(17)])], regs)


def context(r):
    regs = common(r)
    vmax = struct.unpack_from('<i', regs[CINFO], 0xe4)[0]
    height = struct.unpack_from('<I', regs[CINFO], 0x1c)[0]
    row = r.randrange(3 * vmax + 1)
    stop = min(3 * vmax, row + r.choice([0, vmax, 2 * vmax, r.randrange(2 * vmax + 1)]))
    if r.random() < 0.3:
        row, stop = 0, 2 * vmax  # (as start_pass_prep leaves it)
    struct.pack_into('<Iiii', regs[PREP], 0x30, r.choice([0, 1, 2, vmax, height, height, r.randrange(20)]), row,
                     vmax * r.randrange(3), stop)
    avail = r.choice([8, 8, 1, 2])
    in_avail = counters(r, regs, avail)
    return Call(0x654440, [CINFO, INPUT, CTR, in_avail, OPTRS, CTR + 4, avail], regs)


def create(r):
    regs = common(r, r.choice([1, 3, 3, 4, 2, 10]))
    regs[HEAP] = heap(0x1000)
    return Call(0x6546a5, [CINFO], regs)


OPS = [f for f in (jinit, start, data, expand, context, create)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
