# libjpeg 6a's jccoefct.c, the compressor's coefficient buffer controller (src/game/lib/jpeg6a: the library built from
# third_party/jpeg-6a, the module through its wrapper unit third_party/wrap/jpeg-6a/jccoefct.c) against the exe's 6a copies. cinfo at
# CINFO (6a's jpeg_compress_struct: err +0 at ERR, error_exit a cdecl recorder; mem +4 at MEM, alloc_small / alloc_large fncmp's
# ZALLOC over the case heap, request_virt_barray +0x14 a recorder and access_virt_barray +0x20 a recorder answering BARRAY;
# num_components +0x34, comp_info +0x3c at COMPS (component_index +4, h / v_samp_factor +8 / +0xc, width / height_in_blocks +0x1c /
# +0x20, MCU_width / height +0x34 / +0x38, MCU_sample_width +0x40, last_col_width / last_row_height +0x44 / +0x48); total_iMCU_rows
# +0xe8, comps_in_scan +0xec, cur_comp_info[4] +0xf0, MCUs_per_row +0x100; coef +0x150 at COEF (my_coef_controller, 0x68 bytes:
# start_pass, compress_data, iMCU_row_num +8, mcu_ctr +0xc, MCU_vert_offset +0x10, MCU_rows_per_iMCU_row +0x14, MCU_buffer[10] +0x18,
# whole_image[10] +0x40); fdct +0x160 (forward_DCT +4) and entropy +0x164 (encode_mcu +4) in MODS, cdecl recorders, encode_mcu
# answering FALSE after a dealt number of calls (a suspension)). The exe's build has FULL_COEF_BUFFER_SUPPORTED.
# The states stay where the source does: a scan of 1..4 components whose MCU (MCU_width x MCU_height blocks, interleaved: the
# sampling factors; one component: 1 x 1) holds at most ten blocks, last_col_width / last_row_height 1..the MCU's, iMCU_row_num below
# total_iMCU_rows, MCU_rows_per_iMCU_row as start_iMCU_row sets it and the counters below it (a suspension leaves them so).
# compress_data's MCU_buffer is jinit's (ten blocks in BLK); compress_first_pass and compress_output get BARRAY's row pointers to
# BROWS (four rows of eight blocks, as wide as a padded component of width_in_blocks 1..5 at h_samp_factor up to 4).
# usage: jpeg6a_coefct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,row,data,first,output)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fail_after, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); then jutils.c's jround_up and jzero_far
FUNCS = {'jinit_c_coef_controller': (0x650700, 0x650818), 'start_pass_coef': (0x650818, 0x650908),
         'start_iMCU_row': (0x650908, 0x650982), 'compress_data': (0x650982, 0x650c30),
         'compress_first_pass': (0x650c30, 0x650ee3), 'compress_output': (0x650ee3, 0x6510e0)}
CALLED = [(0x6503e3, 0x650401), (0x6504a2, 0x6504bf)]
B = Bench(sorted(FUNCS.values()) + CALLED)

CINFO, ERR, MEM, COEF, MODS, COMPS, BARRAY, INPUT, BLK, BROWS = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x340, RAM + 0x380, RAM + 0x740, RAM + 0x780, RAM + 0x800, RAM + 0x1000)
K_ERROR, K_REQUEST, K_ACCESS, K_FDCT, K_ENCODE = range(5)
ROWBLOCKS = 8  # (blocks a BROWS row)


def common(r, scan=True):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    ncomps = r.choice([1, 3, 3, 4, 2])
    struct.pack_into('<i4xI', cinfo, 0x34, ncomps, COMPS)
    total = r.choice([1, 2, 3, r.randrange(1, 9)])
    nscan = r.choice([1, 1, ncomps, r.randrange(1, min(ncomps, 4) + 1)]) if ncomps <= 4 else r.randrange(1, 5)
    struct.pack_into('<Ii', cinfo, 0xe8, total, nscan)
    comps = bytearray(r.bytes(0x54 * 10))
    for c in range(10):
        h, v = r.choice([1, 2, 1, 2, 4, 3]), r.choice([1, 2, 1, 2, 4])
        struct.pack_into('<iii', comps, 0x54 * c + 4, c, h, v)
        struct.pack_into('<II', comps, 0x54 * c + 0x1c, r.randrange(1, 6), r.choice([1, 2, 3, 4, r.randrange(1, 40)]))
    # the scan's components (distinct) with their MCU sizes, at most ten blocks
    order = list(range(ncomps))
    r.shuffle(order)
    order = sorted(order[:nscan])
    left = 10
    for i, c in enumerate(order):
        h, v = struct.unpack_from('<ii', comps, 0x54 * c + 8)
        if nscan == 1:
            mw = mh = 1
        else:
            while h * v > left - (nscan - 1 - i):
                h, v = max(1, h // 2), max(1, v // 2)
            struct.pack_into('<ii', comps, 0x54 * c + 8, h, v)
            mw, mh = h, v
        left -= mw * mh
        struct.pack_into('<iii', comps, 0x54 * c + 0x34, mw, mh, mw * mh)
        struct.pack_into('<iii', comps, 0x54 * c + 0x40, mw * 8, r.randrange(1, mw + 1),
                         r.randrange(1, (mh if nscan > 1 else v) + 1))
    for i in range(4):
        struct.pack_into('<I', cinfo, 0xf0 + 4 * i, COMPS + 0x54 * (order[i] if i < nscan else r.randrange(10)))
    struct.pack_into('<I', cinfo, 0x100, r.choice([1, 2, 3, r.randrange(1, 7)]))
    struct.pack_into('<I', cinfo, 0x150, COEF)
    struct.pack_into('<II', cinfo, 0x160, MODS, MODS + 0x10)
    mods = bytearray(r.bytes(0x20))
    struct.pack_into('<I', mods, 4, recorder(K_FDCT, 7, cdecl=True))
    struct.pack_into('<I', mods, 0x14, recorder(K_ENCODE, 2, cdecl=True))
    log = log_region()
    set_return(log, K_ACCESS, BARRAY)
    if r.random() < 0.3:
        fail_after(log, K_ENCODE, r.randrange(6), 0)
    mem = bytearray(r.bytes(0x2c))
    struct.pack_into('<II', mem, 0, ZALLOC, ZALLOC)
    struct.pack_into('<I', mem, 0x14, recorder(K_REQUEST, 6, cdecl=True))
    struct.pack_into('<I', mem, 0x20, recorder(K_ACCESS, 5, cdecl=True))
    coef = bytearray(r.bytes(0x68))
    for i in range(10):
        struct.pack_into('<I', coef, 0x18 + 4 * i, BLK + 0x80 * i)
    if r.random() < 0.3:
        struct.pack_into('<I', coef, 0x40, 0)
    if scan:  # the row's counters as start_iMCU_row and a suspension leave them
        row = r.randrange(total)
        if nscan > 1:
            per = 1
        else:
            c0 = struct.unpack_from('<I', cinfo, 0xf0)[0] - COMPS
            per = struct.unpack_from('<i', comps, c0 + (0xc if row < total - 1 else 0x48))[0]
        mcus = struct.unpack_from('<I', cinfo, 0x100)[0]
        struct.pack_into('<IIii', coef, 8, row, r.choice([0, 0, r.randrange(mcus)]), r.choice([0, 0, r.randrange(per)]), per)
    barray = b''.join(struct.pack('<I', BROWS + 0x80 * ROWBLOCKS * (j % 4)) for j in range(8))
    return {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c), MEM: mem, COEF: coef,
            MODS: mods, COMPS: comps, LOG: log, BARRAY: barray, INPUT: r.bytes(0x40), BLK: r.bytes(0x80 * 10),
            BROWS: r.bytes(0x80 * ROWBLOCKS * 4)}


def jinit(r):
    regs = common(r, scan=False)
    regs[HEAP] = heap(0x1000)
    return Call(0x650700, [CINFO, r.choice([0, 0, 1, 1, 0x100])], regs)


def start(r):
    return Call(0x650818, [CINFO, r.choice([0, 1, 2, 3, 0, 2, 3, -1])], common(r))


def row(r):
    return Call(0x650908, [CINFO], common(r))


def data(r):
    return Call(0x650982, [CINFO, INPUT], common(r), ret='a')


def first(r):
    return Call(0x650c30, [CINFO, INPUT], common(r), ret='a')


def output(r):
    return Call(0x650ee3, [CINFO, INPUT], common(r), ret='a')


OPS = [f for f in (jinit, start, row, data, first, output)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
