# libjpeg 6a's jcinit.c jinit_compress_master and jcapistd.c jpeg_start_compress, the compressor's start (src/game/lib/jpeg6a: the
# library built from third_party/jpeg-6a) against the exe's 6a copies. Both run every module's init (jinit_c_master_control,
# jinit_color_converter, jinit_downsampler, jinit_c_prep_controller, jinit_forward_dct, jinit_huff_encoder or jinit_phuff_encoder,
# jinit_c_coef_controller, jinit_c_main_controller, jinit_marker_writer), then the memory manager's realize_virt_arrays and the
# marker writer's write_file_header; jpeg_start_compress also jpeg_suppress_tables, the error manager's reset_error_mgr, the
# destination's init_destination and the master's prepare_for_pass, which runs the first pass's start_pass methods. The library's
# code stores its methods through the method shims, and calls write_file_header and prepare_for_pass, which the inits just stored:
# this bench sets FNCMP_JPEG6A_NATIVE, so the shims leave the library's addresses for the library's side to call and the driver
# gives the exe's back in the heap it reports (Jpeg6a_SetNativeMethods, driver.cpp). The emulator runs the exe's modules.
# cinfo at CINFO (6a's jpeg_compress_struct, 0x168 bytes) as jpeg_set_defaults and jpeg_set_colorspace leave it, with the table
# pointers' sent_table flags random: err +0 at ERR (error_exit, emit_message and reset_error_mgr +0x10 cdecl recorders), mem +4 at
# MEM (alloc_small and alloc_large fncmp's ZALLOC over the case heap; alloc_sarray, alloc_barray, request_virt_sarray / barray and
# realize_virt_arrays recorders: no init reads through the arrays they return), progress +8 at PROGRESS or null, global_state +0x10,
# dest +0x14 at DEST (next_output_byte into OUT, free_in_buffer, init_destination +8 and empty_output_buffer +0xc recorders), the
# image's size, colour spaces (the pairs jccolor.c converts), comp_info at COMPS (sampling factors 1..2, at most 7 blocks an MCU),
# quant tables QT0-QT1 and Huffman tables HT0-HT3 (the other pointers null), scan_info at SCANS (sequential or jpeg_simple_progression's
# shape, as jpeg6a_master.py's unmutated script) or null, the coding options (raw_data_in, optimize_coding, dct_method 0..2,
# restarts, JFIF / Adobe markers).
# ERREXIT returns here (a recorder), so the cases stay where the source, going on, stays in bounds: smoothing_factor 0 and
# CCIR601_sampling off (jcprepct's context buffer would copy row pointers out of alloc_sarray's recorder value), no arithmetic coding
# for jpeg_start_compress (jinit_compress_master's ERREXIT leaves no entropy encoder for prepare_for_pass to start: dealt to
# jinit_compress_master alone, where the module pointers after it stay the case's), and a global_state other than CSTATE_START now
# and then (JERR_BAD_STATE, then on as from the start).
# usage: jpeg6a_init.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=master,start)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder  # noqa: E402
from jpeg6a_huff import STD_BITS  # noqa: E402

os.environ['FNCMP_JPEG6A_NATIVE'] = '1'  # (the driver's: see the header)

# the exe's 6a, all of it (0x64c5f0..0x657cb4: the inits, the methods they store and run)
B = Bench([(0x64c5f0, 0x657cb4)])

CINFO, ERR, MEM, PROGRESS, DEST, COMPS, SCANS = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x2c0, RAM + 0x2e0, RAM + 0x300, RAM + 0x500
QT = [RAM + 0x900, RAM + 0x990]
HT = [RAM + 0xa40 + 0x120 * i for i in range(4)]
OUT = RAM + 0xf00
K_ERROR, K_EMIT, K_RESET, K_INIT, K_EMPTY, K_SARRAY, K_BARRAY, K_VSARRAY, K_VBARRAY, K_REALIZE = range(10)
# (in_color_space, input_components, jpeg_color_space, num_components): the conversions jinit_color_converter takes
SPACES = [(1, 1, 1, 1), (2, 3, 1, 1), (3, 3, 1, 1), (2, 3, 2, 3), (2, 3, 3, 3), (2, 3, 3, 3), (3, 3, 3, 3), (4, 4, 4, 4),
          (4, 4, 5, 4), (5, 4, 5, 4), (0, 1, 0, 1), (0, 2, 0, 2), (0, 3, 0, 3), (0, 4, 0, 4)]


def script(r, ncomps):
    """Sequential scans (the components in one or more), or jpeg_simple_progression's shape; valid for validate_script."""
    scans = []
    if r.random() < 0.4:
        order = list(range(ncomps))
        while order:
            n = min(len(order), r.choice([1, 2, 3, 4]))
            scans.append([n, order[:n], 0, 63, 0, 0])
            order = order[n:]
    else:
        scans.append([ncomps, list(range(ncomps)), 0, 0, 0, 1])
        for c in range(ncomps):
            scans.append([1, [c], 1, 5, 0, 2])
            scans.append([1, [c], 6, 63, 0, 2])
        scans.append([ncomps, list(range(ncomps)), 0, 0, 1, 0])
        for c in range(ncomps):
            scans.append([1, [c], 1, 63, 2, 1])
    out = bytearray()
    for n, idx, ss, se, ah, al in scans:
        out += struct.pack('<5i4i', n, *(list(idx) + [0] * (4 - len(idx))), ss, se, ah, al)
    return out, len(scans)


def htbl(r):
    """A JHUFF_TBL (0x118 bytes): a standard table's bits, huffval random, sent_table random."""
    return bytes(r.choice(STD_BITS)) + r.bytes(256) + r.bytes(3) + struct.pack('<I', r.choice([0, 1]))


def common(r, arith=False):
    cinfo = bytearray(r.bytes(0x168))
    struct.pack_into('<IIIIi', cinfo, 0, ERR, MEM, PROGRESS if r.random() < 0.5 else 0, r.getrandbits(32),
                     100 if r.random() < 0.95 else r.choice([0, 101, 200]))
    struct.pack_into('<I', cinfo, 0x14, DEST)
    inspace, inputc, space, ncomps = r.choice(SPACES)
    struct.pack_into('<IIii', cinfo, 0x18, r.choice([1, 8, 17, r.randrange(1, 300)]), r.choice([1, 8, 16, r.randrange(1, 300)]),
                     inputc, inspace)
    struct.pack_into('<iiiI', cinfo, 0x30, 8, ncomps, space, COMPS)
    struct.pack_into('<4I', cinfo, 0x40, QT[0], QT[1], 0, 0)
    struct.pack_into('<8I', cinfo, 0x50, HT[0], HT[1], 0, 0, HT[2], HT[3], 0, 0)
    comps = bytearray(r.bytes(0x54 * ncomps))
    for c in range(ncomps):
        h, v = (r.choice([1, 1, 2]), r.choice([1, 1, 2])) if c == 0 else (1, 1)
        struct.pack_into('<iiiiii', comps, 0x54 * c + 4, c, h, v, r.randrange(2), r.randrange(2), r.randrange(2))
    regs = {CINFO: cinfo, COMPS: comps}
    if r.random() < 0.5:
        scans, nscans = script(r, ncomps)
        regs[SCANS] = scans
        struct.pack_into('<iI', cinfo, 0xa0, nscans, SCANS)
    else:
        struct.pack_into('<iI', cinfo, 0xa0, r.choice([0, 1]), 0)
    struct.pack_into('<iiiiiiii', cinfo, 0xa8, r.random() < 0.1, 1 if arith else 0, r.choice([0, 0, 1]), 0, 0,
                     r.choice([0, 0, 1, 2]), r.choice([0, 0, 0, 1, r.randrange(1, 100)]), r.choice([0, 0, 0, 1, r.randrange(1, 4)]))
    struct.pack_into('<iBxHHxxi', cinfo, 0xc8, r.choice([0, 1]), r.randrange(3), r.randrange(0x10000), r.randrange(0x10000),
                     r.choice([0, 0, 1]))
    for i, t in enumerate(QT):
        regs[t] = struct.pack('<64H', *[r.randrange(1, 256) for _ in range(64)]) + struct.pack('<I', r.choice([0, 1]))
    for t in HT:
        regs[t] = htbl(r)
    log = log_region()
    err = bytearray(r.bytes(0x80))
    struct.pack_into('<II', err, 0, recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True))
    struct.pack_into('<I', err, 0x10, recorder(K_RESET, 1, cdecl=True))
    mem = struct.pack('<II', ZALLOC, ZALLOC) + struct.pack(
        '<5I', recorder(K_SARRAY, 4, cdecl=True), recorder(K_BARRAY, 4, cdecl=True), recorder(K_VSARRAY, 6, cdecl=True),
        recorder(K_VBARRAY, 6, cdecl=True), recorder(K_REALIZE, 1, cdecl=True)) + r.bytes(0x14)
    dest = bytearray(r.bytes(0x14))
    off = r.randrange(0x20)
    struct.pack_into('<IIII', dest, 0, OUT + off, r.choice([1, 2, 5, 0x100]), recorder(K_INIT, 1, cdecl=True),
                     recorder(K_EMPTY, 1, cdecl=True))
    regs.update({ERR: err, MEM: mem, PROGRESS: r.bytes(0x14), DEST: dest, OUT: r.bytes(0x140), LOG: log, HEAP: heap(0x6000)})
    return regs


def master(r):
    regs = common(r, arith=r.random() < 0.05)
    return Call(0x6500f0, [CINFO], regs)


def start(r):
    regs = common(r)
    return Call(0x64ca90, [CINFO, r.choice([0, 1, 1])], regs)


OPS = [f for f in (master, start) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
