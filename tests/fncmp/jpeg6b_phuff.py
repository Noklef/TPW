# libjpeg 6b's jdphuff.c, the progressive Huffman decoder (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b) against the
# exe's copies. The case layout and the Huffman tables / bit streams are jpeg6b_huff.py's (its header), with the progressive
# decoder's entropy at ENTROPY (phuff_entropy_decoder, 0x40 bytes: pub (start_pass, decode_mcu, insufficient_data +8), bitstate
# (get_buffer +0xc, bits_left +0x10), saved (EOBRUN +0x14, last_dc_val[4] +0x18), restarts_to_go +0x28, derived_tbls[4] +0x2c,
# ac_derived_tbl +0x3c) and cinfo's coef_bits +0x8c at COEFBITS (num_components +0x24 rows of 64 ints), each scan component's
# component_index +4 a row of it.
# start_pass checks the scan's Ss / Se / Ah / Al (the four progressions, sometimes a bad one: ERREXIT4 then on), warns of each
# coefficient whose coef_bits is not Ah, and builds the scan's tables. The four decode_mcu methods run on an MCU (DC: up to 10 blocks
# of up to 4 components, through derived_tbls[dc_tbl_no], the DC table; AC: MCU_data[0] alone, through ac_derived_tbl, the AC one)
# whose blocks are sparse or dense (refinement reads a correction bit for each nonzero coefficient, and counts zero ones), with an
# EOB run pending or not and a restart due or not.
# usage: jpeg6b_phuff.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=jinit,start,dc_first,ac_first,dc_refine,ac_refine,restart)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, Bench, Call, heap, set_return  # noqa: E402
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from jpeg6b_huff import (AC_TBLS, BLOCKS, CINFO, COMP, COMP_SIZE, COMPS_IN_SCAN, DC_TBLS, DTBL, DTBL2, DTBL_SIZE, ENTROPY,  # noqa: E402
                         HTBL, HTBL_SIZE, K_FILL, MCUDATA, MORE, RESTART_INTERVAL, SRC, SS, UNREAD_MARKER, common, derived, htbl_bytes, mcu_tables, scan,
                         source)

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jinit_phuff_decoder': (0x644d30, 0x644db0), 'start_pass_phuff_decoder': (0x644db0, 0x645030),
         'decode_mcu_DC_first': (0x645030, 0x645270), 'process_restart': (0x645270, 0x645300),
         'decode_mcu_AC_first': (0x645300, 0x645560), 'decode_mcu_DC_refine': (0x645560, 0x645660),
         'decode_mcu_AC_refine': (0x645660, 0x645a80),
         # (what they call: jdhuff.c's leaves)
         'jpeg_make_d_derived_tbl': (0x6441f0, 0x6444d0), 'jpeg_fill_bit_buffer': (0x6444d0, 0x6445e0),
         'jpeg_huff_decode': (0x6445e0, 0x6446d0)}
B = Bench(sorted(FUNCS.values()))

COEFBITS = RAM + 0x5000
NUM_COMPONENTS, COEF_BITS = 0x24, 0x8c
ENTROPY_SIZE = 0x40


def entropy_bytes(r):
    e = bytearray(r.bytes(ENTROPY_SIZE))
    e[8] = r.choice([0, 0, 0, 1])  # insufficient_data
    struct.pack_into('<i', e, 0x10, r.choice([0, 1, 8, 16, 25, r.randrange(0, 26)]))  # bits_left
    struct.pack_into('<I', e, 0x14, r.choice([0, 0, 0, 1, 2, r.randrange(0, 100)]))  # EOBRUN
    struct.pack_into('<i', e, 0x28, r.choice([0, 0, 1, r.randrange(0, 50)]))  # restarts_to_go
    return e


def progression(r):
    """Ss, Se, Ah, Al: one of the four progressions (Al the successive approximation bit), sometimes a bad one."""
    al = r.choice([0, 0, 1, 2, r.randrange(0, 14)])
    ah = 0 if r.random() < 0.5 else al + 1
    if r.random() < 0.5:
        ss, se = 0, 0
    else:
        ss = r.randrange(1, 64)
        se = r.randrange(ss, 64)
    if r.random() < 0.1:  # (a bad one: ERREXIT4, which goes on)
        k = r.randrange(4)
        if k == 0:
            se = r.randrange(ss, 64) if ss else r.randrange(1, 64)
        elif k == 1:
            ah = al + r.choice([0, 2])
        elif k == 2:
            al, ah = r.choice([14, 15]), 0
        else:
            ss, se = r.randrange(1, 64), r.randrange(0, 64)
    return ss, se, ah, al


def setup(r):
    """The case, a scan's components (component_index a coef_bits row) and coef_bits; the number of blocks."""
    regions = common(r)
    cinfo = regions[CINFO]
    ss, se, ah, al = progression(r)
    struct.pack_into('<iiii', cinfo, SS, ss, se, ah, al)
    nblocks, member = scan(r, regions)
    ncomp = struct.unpack_from('<i', cinfo, COMPS_IN_SCAN)[0]
    if ss and r.random() < 0.9:  # (an AC scan has one component)
        struct.pack_into('<i', cinfo, COMPS_IN_SCAN, 1)
        ncomp = 1
    num_components = r.choice([ncomp, 3, 4])
    comps = regions[COMP]
    for ci in range(ncomp):
        struct.pack_into('<i', comps, COMP_SIZE * ci + 4, r.randrange(num_components))
    struct.pack_into('<i', cinfo, NUM_COMPONENTS, num_components)
    struct.pack_into('<I', cinfo, COEF_BITS, COEFBITS)
    regions[COEFBITS] = struct.pack('<%di' % (64 * num_components),
                                    *[r.choice([-1, -1, 0, ah, ah, r.randrange(0, 14)]) for _ in range(64 * num_components)])
    return regions, nblocks


def jinit(r):
    regions = common(r)
    struct.pack_into('<i', regions[CINFO], NUM_COMPONENTS, r.choice([1, 3, 4, r.randrange(0, 11)]))
    regions[HEAP] = heap(0xc00)
    return Call(0x644d30, [CINFO], regions)


def start(r):
    regions, _ = setup(r)
    cinfo = regions[CINFO]
    tables = bytearray()
    for i in range(8):
        tables += htbl_bytes(r, i < 4)[0]
    struct.pack_into('<4I', cinfo, DC_TBLS, *[HTBL + HTBL_SIZE * i for i in range(4)])
    struct.pack_into('<4I', cinfo, AC_TBLS, *[HTBL + HTBL_SIZE * (4 + i) for i in range(4)])
    e = entropy_bytes(r)
    struct.pack_into('<4I', e, 0x2c, *[0 if r.random() < 0.7 else DTBL for i in range(4)])
    regions.update({ENTROPY: e, HTBL: tables, DTBL: r.bytes(DTBL_SIZE), HEAP: heap(0x1800)})
    return Call(0x644db0, [CINFO], regions)


def blocks(r):
    density = r.choice([0.05, 0.3, 0.9])
    vals = [0 if r.random() > density else r.choice([1, -1, 2, -2, r.randrange(-0x8000, 0x8000)]) for _ in range(64 * 10)]
    return struct.pack('<640h', *vals)


def refine_tables(r, regions):
    """An AC refinement's table, as an encoder makes it: mostly symbols of size 1 (run << 4 | 1) and EOB runs (r << 4)."""
    raw_ac, bits, vals = htbl_bytes(r, False, False)
    vals = [v if r.random() < 0.1 else (v & 0xf0) | (1 if r.random() < 0.8 else 0) for v in vals]
    regions[HTBL] = regions[HTBL][:HTBL_SIZE] + bytes(bits) + bytes(vals) + raw_ac[17 + len(vals):]
    regions[DTBL2] = derived(bits, HTBL + HTBL_SIZE, vals)


def run_dry(r, regions, e):
    """A source that runs dry mid-block and suspends: a full bit buffer, a few more bytes without a marker (a marker, pending or met,
    fills zeros instead), then FALSE; no EOB run and data left; a long AC band. An AC refinement then stops after it made coefficients
    nonzero, which undoit takes back."""
    cinfo = regions[CINFO]
    struct.pack_into('<i', cinfo, UNREAD_MARKER, 0)
    al = r.randrange(0, 3)
    struct.pack_into('<iiii', cinfo, SS, r.randrange(1, 10), 63, al + 1, al)
    regions[MORE] = bytes(r.randrange(0, 0xff) for _ in range(len(regions[MORE])))
    src = bytearray(regions[SRC])
    struct.pack_into('<I', src, 4, r.randrange(0, 8))
    regions[SRC] = src
    set_return(regions[LOG], K_FILL, 0)
    e[8] = 0
    struct.pack_into('<IiI', e, 0xc, r.getrandbits(32), r.randrange(8, 26), 0)


def decode(address):
    def op(r):
        regions, nblocks = setup(r)
        source(r, regions)
        struct.pack_into('<I', regions[CINFO], RESTART_INTERVAL, r.choice([0, 0, 1, r.randrange(1, 100)]))
        mcu_tables(r, regions)
        if address == 0x645660 and r.random() < 0.6:
            refine_tables(r, regions)
        e = entropy_bytes(r)
        if r.random() < 0.3:
            run_dry(r, regions, e)
        struct.pack_into('<4I', e, 0x2c, DTBL, DTBL, DTBL, DTBL)
        struct.pack_into('<I', e, 0x3c, DTBL2)
        regions.update({ENTROPY: e, MCUDATA: struct.pack('<10I', *[BLOCKS + 0x80 * b for b in range(10)]), BLOCKS: blocks(r)})
        return Call(address, [CINFO, MCUDATA], regions, ret='b')
    return op


def restart(r):
    regions = common(r)
    regions[ENTROPY] = entropy_bytes(r)
    struct.pack_into('<i', regions[ENTROPY], 0x10, r.choice([0, 7, 8, 9, 31, -1, -8, -9, r.randrange(-40, 40)]))
    struct.pack_into('<i', regions[CINFO], COMPS_IN_SCAN, r.choice([0, 1, 3, 4]))
    return Call(0x645270, [CINFO], regions, ret='b')


dc_first, ac_first, dc_refine, ac_refine = decode(0x645030), decode(0x645300), decode(0x645560), decode(0x645660)
dc_first.__name__, ac_first.__name__, dc_refine.__name__, ac_refine.__name__ = 'dc_first', 'ac_first', 'dc_refine', 'ac_refine'
OPS = [f for f in (jinit, start, dc_first, ac_first, dc_refine, ac_refine, restart)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
