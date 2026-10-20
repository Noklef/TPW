# libjpeg 6b's jdmainct.c, the decompression main buffer controller (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b)
# against the exe's copies. cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder (slot 0), msg_code +0x14; mem +4 at MEM:
# alloc_small ZALLOC over the case heap, alloc_sarray +8 a cdecl recorder (slot 1, its answer a hash); num_components +0x24, comp_info
# +0xc4 at COMP (v_samp_factor +0xc, width_in_blocks +0x1c, DCT_scaled_size +0x24, downsampled_height +0x2c; 0x54 bytes each);
# min_DCT_scaled_size +0x118, total_iMCU_rows +0x11c; main +0x184 at MAIN (my_main_controller: pub (start_pass, process_data),
# buffer[10] +8, buffer_full +0x30, rowgroup_ctr +0x34, xbuffer[2] +0x38, whichptr +0x40, context_state +0x44, rowgroups_avail +0x48,
# iMCU_row_ctr +0x4c; 0x50 bytes); coef +0x188 at COEF (decompress_data +0xc a cdecl recorder, slot 2, its answer 0 or 1); post +0x18c
# at POST (post_process_data +4 a cdecl recorder, slot 3, which writes a row group count through &main->rowgroup_ctr and a row count
# through out_row_ctr, as the postprocessor advances them); upsample +0x1a0 at UPS (need_context_rows +8)).
# The context case's pointer lists are laid out as alloc_funny_pointers makes them: for each component, a block of
# 2 * rgroup * (M + 4) row pointers at XB, xbuffer[0][ci] rgroup pointers into it, xbuffer[1][ci] rgroup * (M + 4) past that, with
# the lists themselves (xbuffer[0], then xbuffer[1]) at XL; main->buffer[ci] lists of rgroup * (M + 2) row pointers at BUFS. M is
# min_DCT_scaled_size, rgroup v_samp_factor * DCT_scaled_size / M.
# usage: jpeg6b_mainct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,alloc,start,funny,wrap,bottom,simple,context,crank)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, fill, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_d_main_controller': (0x642b30, 0x642c10), 'alloc_funny_pointers': (0x642c10, 0x642cc0),
         'start_pass_main': (0x642cc0, 0x642d40), 'make_funny_pointers': (0x642d40, 0x642ea0),
         'process_data_simple_main': (0x642ea0, 0x642f10), 'process_data_context_main': (0x642f10, 0x643050),
         'set_wraparound_pointers': (0x643050, 0x643170), 'set_bottom_pointers': (0x643170, 0x643210),
         'process_data_crank_post': (0x643210, 0x643240)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, MAIN, COEF, POST, UPS, ARGS, COMP, XL, BUFS, XB = (RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x360,
                                                                    RAM + 0x380, RAM + 0x3a0, RAM + 0x3c0, RAM + 0x400, RAM + 0x600,
                                                                    RAM + 0x1000, RAM + 0x2000)
K_ERROR, K_ALLOC_SARRAY, K_DECOMPRESS, K_POST = 0, 1, 2, 3
NUM_COMPONENTS, COMP_INFO, MIN_DCT, TOTAL_IMCU_ROWS, MAIN_, COEF_, POST_, UPSAMPLE_ = 0x24, 0xc4, 0x118, 0x11c, 0x184, 0x188, 0x18c, 0x1a0
COMP_SIZE = 0x54


def layout(r, context=True):
    """The case's regions: cinfo, its components and main controller (the pointer lists laid out as above); M; the rgroups."""
    m = r.choice([1, 2, 2, 4, 8]) if context else r.choice([1, 2, 4, 8])
    if context and m < 2:  # (make_funny_pointers would read before buffer[ci]: jinit's ERREXIT case only, below)
        m = 2
    ncomp = r.choice([1, 3, 3, r.randrange(1, 5)])
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<i', cinfo, NUM_COMPONENTS, ncomp)
    struct.pack_into('<I', cinfo, COMP_INFO, COMP)
    struct.pack_into('<iI', cinfo, MIN_DCT, m, r.choice([1, 2, 5, r.randrange(1, 100)]))
    struct.pack_into('<III', cinfo, MAIN_, MAIN, COEF, POST)
    struct.pack_into('<I', cinfo, UPSAMPLE_, UPS)
    comps, rgroups = bytearray(), []
    for ci in range(ncomp):
        c = bytearray(r.bytes(COMP_SIZE))
        v = r.choice([1, 1, 2]) if ci else r.choice([1, 2])
        dct = m * r.choice([1, 1, 2]) if m < 8 else 8
        struct.pack_into('<i', c, 0xc, v)
        struct.pack_into('<i', c, 0x1c, r.randrange(1, 300))
        struct.pack_into('<i', c, 0x24, dct)
        struct.pack_into('<I', c, 0x2c, r.choice([v * dct, v * dct + 1, r.randrange(1, 5000)]))
        comps += c
        rgroups.append(v * dct // m)
    main = bytearray(r.bytes(0x50))
    xl0, xl1, bufs, xb = [], [], [], bytearray()
    b_off = 0
    for ci, rg in enumerate(rgroups):
        base = XB + len(xb)
        xb += r.bytes(4 * 2 * rg * (m + 4))
        xl0.append(base + 4 * rg)
        xl1.append(base + 4 * rg * (m + 5))
        bufs.append(BUFS + b_off)
        b_off += 4 * rg * (m + 2)
    struct.pack_into('<%dI' % ncomp, main, 8, *bufs)
    struct.pack_into('<II', main, 0x38, XL, XL + 4 * ncomp)
    struct.pack_into('<ii', main, 0x40, r.choice([0, 1]), r.choice([0, 1, 2, 2]))
    struct.pack_into('<I', main, 0x4c, r.choice([0, 1, 2, r.randrange(0, 100)]))
    main[0x30] = r.choice([0, 1])
    struct.pack_into('<I', main, 0x34, r.choice([0, m - 1, m, m + 1, r.randrange(0, 12)]) & 0xffffffff)
    struct.pack_into('<I', main, 0x48, r.choice([m - 1, m, m + 2, r.randrange(0, 12)]) & 0xffffffff)
    log = log_region()
    regions = {CINFO: cinfo, ERR: struct.pack('<I', recorder(K_ERROR, 1, cdecl=True)) + r.bytes(0x7c), LOG: log, COMP: comps,
               MEM: struct.pack('<II', ZALLOC, r.getrandbits(32)) + struct.pack('<I', recorder(K_ALLOC_SARRAY, 4, cdecl=True)) +
               r.bytes(0x28),
               MAIN: main, COEF: r.bytes(0xc) + struct.pack('<I', recorder(K_DECOMPRESS, 2, cdecl=True)) + r.bytes(0x10),
               POST: r.bytes(4) + struct.pack('<I', recorder(K_POST, 7, cdecl=True)) + r.bytes(0x8),
               UPS: r.bytes(8) + bytes([r.choice([0, 1])]) + r.bytes(7),
               XL: struct.pack('<%dI' % (2 * ncomp), *(xl0 + xl1)), BUFS: r.bytes(max(b_off, 4)), XB: xb}
    return regions, m, rgroups


def jinit(r):
    regions, m, _ = layout(r, r.random() < 0.5)
    if r.random() < 0.1:  # (M 1 with context rows: ERREXIT JERR_NOTIMPL, which goes on)
        struct.pack_into('<i', regions[CINFO], MIN_DCT, 1)
    regions[HEAP] = heap(0x1000)
    return Call(0x642b30, [CINFO, r.choice([0, 0, 0, 1]) | (r.randrange(256) << 8 if r.wild else 0)], regions)


def alloc(r):
    regions, m, _ = layout(r)
    regions[HEAP] = heap(0x1000)
    return Call(0x642c10, [CINFO], regions)


def start(r):
    regions, m, _ = layout(r)
    return Call(0x642cc0, [CINFO, r.choice([0, 0, 0, 2, 2, 1, 3, r.randrange(-2, 6)])], regions)


def local(address):
    def op(r):
        regions, m, _ = layout(r)
        return Call(address, [CINFO], regions)
    return op


def process(address):
    def op(r):
        regions, m, _ = layout(r)
        log = regions[LOG]
        if r.random() < 0.15:
            set_return(log, K_DECOMPRESS, 0)  # (suspended)
        else:
            set_return(log, K_DECOMPRESS, r.choice([1, 1, 2, 3]))
        out_avail = r.choice([0, 1, 8, r.randrange(0, 64)])
        out_ctr = r.choice([0, r.randrange(0, out_avail + 1)])
        # (the postprocessor: the row group counter it was handed, and the output rows)
        fill(log, K_POST, 2, struct.pack('<I', r.choice([0, m - 1, m, m + 1, m + 2, r.randrange(0, 12)]) & 0xffffffff))
        fill(log, K_POST, 5, struct.pack('<I', r.choice([out_ctr, out_avail, r.randrange(0, 70)])), 1)
        regions[ARGS] = struct.pack('<I', out_ctr) + r.bytes(4)
        return Call(address, [CINFO, r.getrandbits(32), ARGS, out_avail], regions)
    return op


funny, wrap, bottom = local(0x642d40), local(0x643050), local(0x643170)
simple, context, crank = process(0x642ea0), process(0x642f10), process(0x643210)
for f, n in ((funny, 'funny'), (wrap, 'wrap'), (bottom, 'bottom'), (simple, 'simple'), (context, 'context'), (crank, 'crank')):
    f.__name__ = n
OPS = [f for f in (jinit, alloc, start, funny, wrap, bottom, simple, context, crank)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
