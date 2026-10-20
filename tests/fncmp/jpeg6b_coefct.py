# libjpeg 6b's jdcoefct.c, the decompression coefficient buffer controller (src/game/lib/jpeg6b: the library built from
# third_party/jpeg-6b) against the exe's copies. cinfo at CINFO (mem +4 at MEM: alloc_small and alloc_large ZALLOC over the case heap,
# request_virt_barray +0x14 a cdecl recorder (slot 0, its answer a hash), access_virt_barray +0x20 one (slot 1) answering ROWS + 0x10;
# num_components +0x24, do_block_smoothing +0x49, input_scan_number +0x7c, input_iMCU_row +0x80, output_scan_number +0x84,
# output_iMCU_row +0x88, coef_bits +0x8c at COEFBITS (int[64] a component), comp_info +0xc4 at COMP (component_index +4, h/v_samp_factor
# +8/+0xc, width/height_in_blocks +0x1c/+0x20, DCT_scaled_size +0x24, component_needed +0x30, MCU_width/height/blocks +0x34/+0x38/+0x3c,
# MCU_sample_width +0x40, last_col_width +0x44, last_row_height +0x48, quant_table +0x4c at QT; 0x54 bytes each), progressive_mode +0xc8,
# total_iMCU_rows +0x11c, comps_in_scan +0x124, cur_comp_info[4] +0x128, MCUs_per_row +0x138, blocks_in_MCU +0x140, Ss +0x16c;
# coef +0x188 at COEF (my_coef_controller: pub (start_input_pass, consume_data, start_output_pass, decompress_data, coef_arrays), MCU_ctr
# +0x14, MCU_vert_offset +0x18, MCU_rows_per_iMCU_row +0x1c, MCU_buffer[10] +0x20 (blocks at MCUB), whole_image[10] +0x48,
# coef_bits_latch +0x70 at LATCH; 0x74 bytes); inputctl +0x190 at INCTL (consume_input +0 a recorder, slot 4, answering JPEG_SUSPENDED:
# the bench cannot advance the scan counters it would; finish_input_pass +0xc, slot 5; eoi_reached +0x11); entropy +0x198 at ENT
# (decode_mcu +4, slot 2, its answer TRUE or FALSE, the MCU_buffer list logged as a hash of its pointers); idct +0x19c at IDCT
# (inverse_DCT[ci] +4 one recorder, slot 3; in the smoothing case the block, a workspace on the stack, is logged as a hash of its
# coefficients)). The virtual arrays' block rows: ROWS row pointers, each W blocks at BLK.
# usage: jpeg6b_coefct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,input,imcu,output,smoothok,onepass,consume,data,smooth)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, blob, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_d_coef_controller': (0x643240, 0x643350), 'start_input_pass': (0x643350, 0x643370),
         'start_iMCU_row': (0x643370, 0x6433d0), 'start_output_pass': (0x6433d0, 0x643420),
         'decompress_onepass': (0x643420, 0x643680), 'consume_data': (0x643680, 0x643890),
         'decompress_data': (0x643890, 0x643a40), 'smoothing_ok': (0x643a40, 0x643b70),
         'decompress_smooth_data': (0x643b70, 0x6441f0)}
# (and jutils.c's jround_up, jcopy_block_row and jzero_far, which jinit, decompress_smooth_data and decompress_onepass call)
B = Bench(sorted(list(FUNCS.values()) + [(0x6428c0, 0x6428e0), (0x642930, 0x642980)]))

CINFO, MEM, COEF, INCTL, ENT, IDCT, OUTB, QT, COMP, COEFBITS, LATCH, ROWS, MCUB, BLK = (
    RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x320, RAM + 0x340, RAM + 0x380, RAM + 0x400, RAM + 0x700, RAM + 0x900,
    RAM + 0xd00, RAM + 0xe00, RAM + 0x1000, RAM + 0x2000)
K_REQUEST, K_ACCESS, K_DECODE, K_IDCT, K_CONSUME, K_FINISH = 0, 1, 2, 3, 4, 5
NROWS, W = 24, 5  # (ROWS: row pointers; each row W blocks)
QT_SIZE, COMP_SIZE = 0x84, 0x54


def coefs(r, block):
    """A block's coefficients: DC, then mostly small values, with the five that smoothing estimates often zero."""
    dc = r.randrange(-32768, 32768) if r.wild and r.random() < 0.3 else r.randrange(-1024, 1024)
    vals = [dc] + [r.choice([0, 0, 0, 1, -1, r.randrange(-64, 64)]) for _ in range(63)]
    for i in (1, 2, 8, 9, 16):
        if r.random() < 0.6:
            vals[i] = 0
    struct.pack_into('<64h', block, 0, *vals)


def layout(r, smooth=False):
    """The case's regions: cinfo, its components (and those in the scan), the controller and the modules it calls."""
    ncomp = r.choice([1, 3, 3, r.randrange(1, 5)])
    cinfo = bytearray(r.bytes(0x1b0))
    total = r.choice([1, 2, 3, r.randrange(1, 40)])
    struct.pack_into('<II', cinfo, 0, r.getrandbits(32), MEM)
    struct.pack_into('<i', cinfo, 0x24, ncomp)
    cinfo[0x49] = r.choice([0, 1, 1])
    out_scan = r.randrange(0, 6)
    struct.pack_into('<iIiI', cinfo, 0x7c, out_scan + r.choice([-1, 0, 0, 1]), r.choice([0, 1, total - 1, total, r.randrange(0, 41)]),
                     out_scan, r.choice([0, 1, total - 2, total - 1, r.randrange(0, 41)]) & 0xffffffff)
    struct.pack_into('<I', cinfo, 0x8c, r.choice([COEFBITS, COEFBITS, COEFBITS, 0]))
    struct.pack_into('<I', cinfo, 0xc4, COMP)
    cinfo[0xc8] = r.choice([0, 1, 1, 1])
    struct.pack_into('<I', cinfo, 0x11c, total)
    comps, qts, blocks_left = bytearray(), bytearray(), 10
    for ci in range(ncomp):
        c = bytearray(r.bytes(COMP_SIZE))
        h, v = r.choice([1, 1, 2]), r.choice([1, 1, 2])
        dct = r.choice([1, 2, 4, 8, 8])
        mw, mh = (r.choice([1, h]), r.choice([1, v])) if blocks_left >= 4 * (4 - min(ci, 3)) else (1, 1)
        struct.pack_into('<iiii', c, 4, ci, h, v, r.randrange(4))
        struct.pack_into('<iii', c, 0x1c, r.randrange(1, W + 1), r.choice([v, v + 1, r.randrange(1, 50)]), dct)
        c[0x30] = r.choice([1, 1, 1, 0])
        struct.pack_into('<iiiiii', c, 0x34, mw, mh, mw * mh, mw * dct, r.randrange(1, mw + 1), r.randrange(1, mh + 1))
        struct.pack_into('<I', c, 0x4c, QT + QT_SIZE * ci if smooth or r.random() < 0.9 else 0)
        comps += c
        q = [r.randrange(1, 65536) if r.wild else r.choice([1, 2, 16, r.randrange(1, 256)]) for _ in range(64)]
        if not smooth and r.random() < 0.15:
            q[r.choice([0, 1, 2, 8, 9, 16, r.randrange(64)])] = 0
        qts += struct.pack('<64H', *q) + r.bytes(4)
    # the scan's components (at most ten blocks to an MCU)
    nscan = r.choice([1, 1, ncomp, r.randrange(1, ncomp + 1)])
    scan = sorted(r.sample(range(ncomp), nscan))
    blocks = 0
    for i, ci in enumerate(scan):
        mw, mh = struct.unpack_from('<ii', comps, COMP_SIZE * ci + 0x34)
        if blocks + mw * mh > 10 - (nscan - i - 1):
            mw = mh = 1
            struct.pack_into('<iii', comps, COMP_SIZE * ci + 0x34, 1, 1, 1)
            struct.pack_into('<ii', comps, COMP_SIZE * ci + 0x44, 1, 1)
        blocks += mw * mh
    struct.pack_into('<i', cinfo, 0x124, nscan)
    struct.pack_into('<4I', cinfo, 0x128, *([COMP + COMP_SIZE * ci for ci in scan] + [r.getrandbits(32)] * (4 - nscan)))
    struct.pack_into('<I', cinfo, 0x138, r.choice([1, 2, 3, r.randrange(1, 4)]))
    struct.pack_into('<i', cinfo, 0x140, r.choice([blocks, blocks, r.randrange(1, blocks + 1)]))
    struct.pack_into('<i', cinfo, 0x16c, r.choice([0, 0, 1, r.randrange(0, 64)]))
    struct.pack_into('<I', cinfo, 0x188, COEF)
    struct.pack_into('<I', cinfo, 0x190, INCTL)
    struct.pack_into('<I', cinfo, 0x198, ENT)
    struct.pack_into('<I', cinfo, 0x19c, IDCT)
    coef = bytearray(r.bytes(0x74))
    struct.pack_into('<I', coef, 0x10, r.choice([0, r.getrandbits(32)]))
    struct.pack_into('<III', coef, 0x14, r.choice([0, 0, 1, r.randrange(0, 4)]), r.choice([0, 0, 1]), r.choice([1, 1, 2]))
    struct.pack_into('<10I', coef, 0x20, *[MCUB + 0x80 * i for i in range(10)])
    struct.pack_into('<I', coef, 0x70, r.choice([LATCH, LATCH, 0]))
    latch = struct.pack('<%di' % (6 * ncomp), *[r.choice([-1, 0, 0, 1, 2, 3, 5, 9, r.randrange(-3, 14)]) for _ in range(6 * ncomp)])
    coefbits = struct.pack('<%di' % (64 * ncomp), *[r.choice([-1, 0, 0, 1, 2, 3, 5, 9, r.randrange(-3, 14)]) for _ in range(64 * ncomp)])
    rows = struct.pack('<%dI' % NROWS, *[BLK + 0x80 * W * (i % 12) for i in range(NROWS)])
    blk = bytearray(0x80 * W * 12)
    for b in range(W * 12):
        coefs(r, memoryview(blk)[0x80 * b:])
    mcub = bytearray(r.bytes(0x500))
    log = log_region()
    regions = {CINFO: cinfo, LOG: log, COMP: comps, QT: qts, COEF: coef, LATCH: latch, COEFBITS: coefbits, ROWS: rows, BLK: blk, MCUB: mcub,
               MEM: struct.pack('<II', ZALLOC, ZALLOC) + r.bytes(0xc) + struct.pack('<I', recorder(K_REQUEST, 6, cdecl=True)) + r.bytes(8) +
               struct.pack('<I', recorder(K_ACCESS, 5, cdecl=True)) + r.bytes(0x1c),
               INCTL: struct.pack('<I', recorder(K_CONSUME, 1, cdecl=True)) + r.bytes(8) + struct.pack('<I', recorder(K_FINISH, 1, cdecl=True)) +
               r.bytes(1) + bytes([r.choice([0, 0, 1])]) + r.bytes(0xe),
               ENT: r.bytes(4) + struct.pack('<I', recorder(K_DECODE, 2, cdecl=True)) + r.bytes(8),
               IDCT: r.bytes(4) + struct.pack('<10I', *[recorder(K_IDCT, 5, cdecl=True)] * 10) + r.bytes(0xc),
               OUTB: r.bytes(0x28)}
    set_return(log, K_ACCESS, ROWS + 0x10)
    set_return(log, K_CONSUME, 0)
    set_return(log, K_DECODE, r.choice([1, 1, 1, 0]))
    return regions


def jinit(r):
    regions = layout(r)
    regions[HEAP] = heap(0x1000)
    return Call(0x643240, [CINFO, r.choice([0, 1]) | (r.randrange(256) << 8 if r.wild else 0)], regions)


def input_pass(r):
    regions = layout(r)
    return Call(0x643350, [CINFO], regions)


def imcu(r):
    regions = layout(r)
    return Call(0x643370, [CINFO], regions)


def output_pass(r):
    regions = layout(r)
    regions[HEAP] = heap(0x1000)
    return Call(0x6433d0, [CINFO], regions)


def smoothok(r):
    regions = layout(r)
    regions[HEAP] = heap(0x1000)
    return Call(0x643a40, [CINFO], regions, ret='b')


def onepass(r):
    regions = layout(r)
    return Call(0x643420, [CINFO, OUTB], regions, ret='a')


def consume(r):
    regions = layout(r)
    blob(regions[LOG], K_DECODE, 1, 0x3ff)  # (the MCU_buffer list: the blocks' pointers)
    return Call(0x643680, [CINFO], regions, ret='a')


def data(r):
    regions = layout(r)
    return Call(0x643890, [CINFO, OUTB], regions, ret='a')


def smooth(r):
    regions = layout(r, smooth=True)
    struct.pack_into('<I', regions[COEF], 0x70, LATCH)  # (start_output_pass latched them: smoothing_ok)
    blob(regions[LOG], K_IDCT, 2, 0xffffffff)  # (the workspace block on the stack)
    return Call(0x643b70, [CINFO, OUTB], regions, ret='a')


input_pass.__name__, output_pass.__name__ = 'input', 'output'
OPS = [f for f in (jinit, input_pass, imcu, output_pass, smoothok, onepass, consume, data, smooth)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
