# libjpeg 6b's jdinput.c, the input controller (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b) against the exe's
# copies. cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder (slot 0), emit_message +4 (slot 1), reset_error_mgr +0x10 (slot 2);
# mem +4 at MEM: alloc_small ZALLOC over the case heap; image_width / height +0x1c / +0x20, num_components +0x24, input_scan_number
# +0x7c, output_scan_number +0x84, coef_bits +0x8c, quant_tbl_ptrs[4] +0x90 at QTBL (and dc_huff_tbl_ptrs[4] +0xa0 there too: an
# out-of-range quant_tbl_no reads past the four), data_precision +0xc0, comp_info +0xc4 at COMP, progressive_mode +0xc8,
# comps_in_scan +0x124, cur_comp_info[4] +0x128 into COMP; coef +0x188 at COEF (start_input_pass a cdecl recorder (slot 6),
# consume_data +4 a value), inputctl +0x190 at INPUTCTL (my_input_controller, 0x18 bytes), marker +0x194 at MARKER (reset_marker_reader
# (slot 3), read_markers +4 (slot 4) answering 0 / 1 / 2 (JPEG_SUSPENDED / REACHED_SOS / REACHED_EOI), saw_SOF +0xd), entropy +0x198 at
# ENTROPY (start_pass a cdecl recorder, slot 5)). The regions after cinfo start at +0x300: per_scan_setup's MCU_membership[10] +0x144 is
# written past its end (to +0x244) when an MCU has too many blocks (JERR_BAD_MCU_SIZE goes on), the same in both.
# usage: jpeg6b_input.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,scan,latch,finish,consume,setup,reset)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end); jdiv_round_up is jutils.c's
FUNCS = {'jinit_input_controller': (0x63b2c0, 0x63b310), 'start_input_pass': (0x63b310, 0x63b360), 'per_scan_setup': (0x63b360, 0x63b510),
         'latch_quant_tables': (0x63b510, 0x63b5c0), 'finish_input_pass': (0x63b5c0, 0x63b5e0), 'consume_markers': (0x63b5e0, 0x63b6b0),
         'initial_setup': (0x63b6b0, 0x63b8b0), 'reset_input_controller': (0x63b8b0, 0x63b8f0), 'jdiv_round_up': (0x6428b0, 0x6428c0)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, INPUTCTL, MARKER, COEF, ENTROPY = RAM, RAM + 0x300, RAM + 0x390, RAM + 0x3c0, RAM + 0x3e0, RAM + 0x490, RAM + 0x4a0
COMP, QTBL = RAM + 0x500, RAM + 0x900
K_ERROR, K_EMIT, K_RESET_ERR, K_RESET_MARKER, K_READ, K_ENTROPY, K_COEF = 0, 1, 2, 3, 4, 5, 6
COMP_SIZE, QTBL_SIZE, QTBL_STRIDE = 0x54, 0x82, 0x84


def common(r):
    log = log_region()
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<8I', cinfo, 0x90, *[QTBL + QTBL_STRIDE * i if r.random() < 0.95 else 0 for i in range(8)])
    struct.pack_into('<I', cinfo, 0x188, COEF)
    struct.pack_into('<III', cinfo, 0x190, INPUTCTL, MARKER, ENTROPY)
    struct.pack_into('<i', cinfo, 0xc0, 8 if r.random() < 0.9 else r.choice([12, 0]))
    cinfo[0xc8] = r.choice([0, 0, 1])
    ctl = bytearray(r.bytes(0x18))
    ctl[0x10], ctl[0x11], ctl[0x14] = r.choice([0, 1]), r.choice([0, 0, 1]), r.choice([0, 1])
    marker = bytearray(r.bytes(0xa8))
    struct.pack_into('<II', marker, 0, recorder(K_RESET_MARKER, 1, cdecl=True), recorder(K_READ, 1, cdecl=True))
    marker[0xd] = r.choice([0, 1])
    set_return(log, K_READ, r.choice([0, 1, 1, 2]))
    return {CINFO: cinfo, LOG: log, INPUTCTL: ctl, MARKER: marker, HEAP: heap(0x400),
            ERR: struct.pack('<5I', *[recorder(k, 2 if k == K_EMIT else 1, cdecl=True) for k in (K_ERROR, K_EMIT, 7, 7, K_RESET_ERR)]) +
            r.bytes(0x70),
            MEM: struct.pack('<I', ZALLOC) + r.bytes(0x1c),
            COEF: struct.pack('<II', recorder(K_COEF, 1, cdecl=True), r.getrandbits(32)) + r.bytes(8),
            ENTROPY: struct.pack('<I', recorder(K_ENTROPY, 1, cdecl=True)) + r.bytes(0xc),
            QTBL: r.bytes(QTBL_STRIDE * 8)}


def image(r, regions):
    """The frame: num_components components (sampling factors 1..4, sometimes not), the image's size, comps_in_scan of them in the scan."""
    cinfo = regions[CINFO]
    n = r.choice([1, 3, 3, 4, r.randrange(1, 11)]) if r.random() < 0.95 else r.choice([0, 11])
    good = r.random() < 0.85
    comps = bytearray(r.bytes(COMP_SIZE * max(n, 1)))
    for i in range(n):
        h, v = (r.choice([1, 1, 2]), r.choice([1, 1, 2])) if i else (r.choice([1, 2, 2, 4]), r.choice([1, 2, 2, 4]))
        if not good and r.random() < 0.3:
            h, v = r.choice([0, 5, -1, h]), r.choice([0, 5, v])
        struct.pack_into('<ii', comps, i * COMP_SIZE + 8, h, v)
        struct.pack_into('<i', comps, i * COMP_SIZE + 0x10, r.randrange(4) if good or r.random() < 0.5 else r.randrange(8))
        struct.pack_into('<I', comps, i * COMP_SIZE + 0x4c, 0 if r.random() < 0.7 else QTBL + QTBL_STRIDE * r.randrange(8))
    struct.pack_into('<ii', cinfo, 0x1c, *[r.choice([1, 7, 8, 9, 16, 17, 600, r.randrange(1, 0x10000)]) if good or r.random() < 0.6
                                            else r.choice([65500, 65501, 0, 0x7fffff]) for _ in range(2)])
    struct.pack_into('<i', cinfo, 0x24, n)
    struct.pack_into('<I', cinfo, 0xc4, COMP)
    regions[COMP] = comps
    return n


def scan(r, regions, n):
    """comps_in_scan (1..4 of the frame's, sometimes 0) and cur_comp_info, with the frame's block sizes as initial_setup leaves them."""
    cinfo, comps = regions[CINFO], regions[COMP]
    m = min(n, r.choice([1, 1, 3, 4, r.randrange(1, 5)])) if r.random() < 0.95 and n else 0
    picks = r.sample(range(n), m) if r.random() < 0.8 else [r.randrange(n) for _ in range(m)]
    struct.pack_into('<i', cinfo, 0x124, m)
    struct.pack_into('<4I', cinfo, 0x128, *[COMP + COMP_SIZE * picks[i] if i < m else r.getrandbits(32) for i in range(4)])
    for i in range(n):
        struct.pack_into('<ii', comps, i * COMP_SIZE + 0x1c, r.choice([1, 2, 5, 75, r.randrange(1, 300)]), r.choice([1, 2, 5, 38, r.randrange(1, 300)]))
        struct.pack_into('<i', comps, i * COMP_SIZE + 0x24, r.choice([8, 8, 4, 2, 1]))
    struct.pack_into('<ii', cinfo, 0x110, r.choice([1, 2, 4]), r.choice([1, 2, 4]))


def jinit(r):
    return Call(0x63b2c0, [CINFO], common(r))


def start(r):
    regions = common(r)
    scan(r, regions, image(r, regions) or 1)
    return Call(0x63b310, [CINFO], regions)


def scan_setup(r):
    regions = common(r)
    scan(r, regions, image(r, regions) or 1)
    return Call(0x63b360, [CINFO], regions)


def latch(r):
    regions = common(r)
    scan(r, regions, image(r, regions) or 1)
    return Call(0x63b510, [CINFO], regions)


def finish(r):
    return Call(0x63b5c0, [CINFO], common(r))


def consume(r):
    regions = common(r)
    scan(r, regions, image(r, regions) or 1)
    struct.pack_into('<ii', regions[CINFO], 0x7c, r.choice([0, 1, 3, r.randrange(10)]), 0)
    struct.pack_into('<i', regions[CINFO], 0x84, r.choice([0, 1, 3, r.randrange(10)]))
    return Call(0x63b5e0, [CINFO], regions, ret='a')


def setup(r):
    regions = common(r)
    n = image(r, regions)
    struct.pack_into('<i', regions[CINFO], 0x124, r.choice([n, n, 1, r.randrange(0, 5)]))
    return Call(0x63b6b0, [CINFO], regions)


def reset(r):
    return Call(0x63b8b0, [CINFO], common(r))


scan_setup.__name__ = 'scan'
OPS = [f for f in (jinit, start, scan_setup, latch, finish, consume, setup, reset)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
