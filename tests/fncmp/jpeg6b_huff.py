# libjpeg 6b's jdhuff.c, the sequential Huffman decoder (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b) against the
# exe's copies. cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder (slot 0), emit_message +4 another (slot 1, (cinfo, level));
# msg_code +0x14, msg_parm +0x18; mem +4 at MEM: alloc_small ZALLOC over the case heap; src +0x18 at SRC: next_input_byte,
# bytes_in_buffer, fill_input_buffer +0xc a cdecl recorder (slot 2) whose answer is a hash (TRUE) or set to 0 (FALSE): the source is
# the game's own (see docs/porting.md), so a "refill" here hands back what src points at, a second buffer at MORE; the DC / AC tables
# dc_huff_tbl_ptrs[4] +0xa0, ac +0xb0 at HTBL; restart_interval +0xfc; comps_in_scan +0x124, cur_comp_info[4] +0x128 at COMP;
# blocks_in_MCU +0x140, MCU_membership[10] +0x144; Ss / Se / Ah / Al +0x16c..; unread_marker +0x17c; marker +0x194 at MARKER
# (read_restart_marker +8 a cdecl recorder (slot 3), discarded_bytes +0x14); entropy +0x198 at ENTROPY).
# The entropy decoder (huff_entropy_decoder, 0xac bytes): pub (start_pass, decode_mcu, insufficient_data +8), bitstate (get_buffer +0xc,
# bits_left +0x10), saved.last_dc_val[4] +0x14, restarts_to_go +0x24, dc_derived_tbls[4] +0x28, ac +0x38, dc_cur_tbls[10] +0x48,
# ac +0x70, dc_needed[10] +0x98, ac_needed[10] +0xa2.
# jpeg_make_d_derived_tbl builds a d_derived_tbl (0x590 bytes: maxcode[18], valoffset[17], pub +0x8c, look_nbits[256] +0x90,
# look_sym[256] +0x490) from a JHUFF_TBL (bits[17], huffval[256]); the tables here are valid Huffman codes (the code space left
# free), sometimes with symbols over 15 for a DC table or a code that does not fit (both error paths, which go on). Its locals
# (huffsize[257], huffcode[257]) are written before they are read for any table whose counts sum to 256 or less, which all of these
# do (a larger sum overruns them in both, past what a bench can compare). jpeg_fill_bit_buffer and jpeg_huff_decode take a
# bitread_working_state at STATE (next_input_byte, bytes_in_buffer, get_buffer, bits_left, cinfo) and the bytes at BUF: 0xFF 0x00
# stuffing, markers (0xFF then another byte: unread_marker, then zeros), and FF FF runs. jpeg_huff_decode's table is a
# d_derived_tbl computed here as jpeg_make_d_derived_tbl makes it (its maxcode[17] the 0xFFFFF sentinel); about half its cases put one
# of the table's codes (a long one, more often than not) next in the bit buffer. decode_mcu decodes an MCU of up to 10 blocks into
# the blocks at BLOCKS (MCU_data at MCUDATA) through a DC and an AC table computed the same way (DC symbols 0..15, as
# jpeg_make_d_derived_tbl insists), with a restart (process_restart) when one is due.
# usage: jpeg6b_huff.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=derived,fill,decode,jinit,start,mcu,restart)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_make_d_derived_tbl': (0x6441f0, 0x6444d0), 'jpeg_fill_bit_buffer': (0x6444d0, 0x6445e0),
         'jpeg_huff_decode': (0x6445e0, 0x6446d0), 'jinit_huff_decoder': (0x6446d0, 0x644710),
         'start_pass_huff_decoder': (0x644710, 0x644860), 'decode_mcu': (0x644860, 0x644cb0), 'process_restart': (0x644cb0, 0x644d30)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, SRC, STATE, PDTBL, HTBL, DTBL, BUF, MORE = (RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x360, RAM + 0x380,
                                                             RAM + 0x400, RAM + 0x1000, RAM + 0x2000, RAM + 0x2100)
ENTROPY, MARKER, COMP, MCUDATA, BLOCKS, DTBL2 = RAM + 0x3000, RAM + 0x3100, RAM + 0x3200, RAM + 0x3400, RAM + 0x3500, RAM + 0x4000
SRC_, DC_TBLS, AC_TBLS, RESTART_INTERVAL, COMPS_IN_SCAN, CUR_COMP_INFO = 0x18, 0xa0, 0xb0, 0xfc, 0x124, 0x128
BLOCKS_IN_MCU, MCU_MEMBERSHIP, SS, UNREAD_MARKER, MARKER_, ENTROPY_ = 0x140, 0x144, 0x16c, 0x17c, 0x194, 0x198
K_ERROR, K_EMIT, K_FILL, K_RESTART = 0, 1, 2, 3
HTBL_SIZE = 0x114  # JHUFF_TBL: bits[17], huffval[256], sent_table (and padding)
DTBL_SIZE, ENTROPY_SIZE, COMP_SIZE, MORE_SIZE = 0x590, 0xac, 0x54, 0x400


def huff_bits(r, overfull=True):
    """bits[1..16] of a valid code (some code space always left free, at most 256 symbols), occasionally an over-full one."""
    bits, space, total = [0] * 17, 1, 0
    top = r.choice([4, 8, 12, 16, 16])
    for l in range(1, top + 1):
        space *= 2
        c = min(r.randrange(0, space), 256 - total)
        if r.random() < 0.5:
            c = min(c, r.randrange(0, 6))
        bits[l], space, total = c, space - c, total + c
    if overfull and r.random() < 0.05:  # (a code past its length: the error path)
        l = r.randrange(1, 9)
        bits[l] = min(bits[l] + (1 << l), bits[l] + 256 - sum(bits))
    return bits


def htbl_bytes(r, dc, overfull=True, dc_strict=False):
    bits = huff_bits(r, overfull)
    while not sum(bits) and not overfull:
        bits = huff_bits(r, overfull)
    n = sum(bits)
    vals = [r.randrange(0, 16) if dc and (dc_strict or r.random() < 0.95) else r.randrange(256) for _ in range(n)]
    if not dc and n and r.random() < 0.7:
        vals[r.randrange(n)] = 0  # (an EOB, so that most blocks end before their 63rd coefficient)
    return bytes(bits) + bytes(vals) + r.bytes(256 - n) + r.bytes(HTBL_SIZE - 17 - 256), bits, vals


def codes_of(bits):
    """The canonical code of each symbol, with its length."""
    sizes = [l for l in range(1, 17) for _ in range(bits[l])]
    codes, code, si, p = [], 0, sizes[0] if sizes else 0, 0
    while p < len(sizes):
        while p < len(sizes) and sizes[p] == si:
            codes.append(code)
            code += 1
            p += 1
        code <<= 1
        si += 1
    return codes, sizes


def derived(bits, htbl_address, vals=None, r=None):
    """jpeg_make_d_derived_tbl's table: maxcode, valoffset and pub; with vals, the lookahead tables too (else r's random bytes, which
    jpeg_huff_decode does not read)."""
    codes, sizes = codes_of(bits)
    maxcode, valoffset, p = [0] * 18, [0] * 17, 0
    for l in range(1, 17):
        if bits[l]:
            valoffset[l] = p - codes[p]
            p += bits[l]
            maxcode[l] = codes[p - 1]
        else:
            maxcode[l] = -1
    maxcode[17] = 0xfffff
    out = struct.pack('<18i', *maxcode) + struct.pack('<17i', *valoffset) + struct.pack('<I', htbl_address)
    if vals is None:
        return out + r.bytes(DTBL_SIZE - 0x90)
    look_nbits, look_sym = [0] * 256, bytearray(256)
    for p, (code, l) in enumerate(zip(codes, sizes)):
        if l <= 8:
            look = code << (8 - l)
            for i in range(1 << (8 - l)):
                look_nbits[look + i], look_sym[look + i] = l, vals[p]
    return out + struct.pack('<256i', *look_nbits) + bytes(look_sym)


def cinfo_bytes(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, SRC_, SRC)
    struct.pack_into('<I', cinfo, ENTROPY_, ENTROPY)
    struct.pack_into('<I', cinfo, MARKER_, MARKER)
    struct.pack_into('<i', cinfo, UNREAD_MARKER, 0 if r.random() < 0.8 else r.randrange(1, 256))
    return cinfo


def common(r):
    log = log_region()
    if r.random() < 0.15:
        set_return(log, K_FILL, 0)  # (fill_input_buffer suspends)
    if r.random() < 0.2:
        set_return(log, K_RESTART, 0)  # (read_restart_marker suspends)
    return {CINFO: cinfo_bytes(r), ERR: struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x78),
            MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), LOG: log,
            MARKER: r.bytes(8) + struct.pack('<I', recorder(K_RESTART, 1, cdecl=True)) + r.bytes(0x14)}


def entropy_bytes(r):
    e = bytearray(r.bytes(ENTROPY_SIZE))
    e[8] = r.choice([0, 0, 0, 1])  # insufficient_data
    struct.pack_into('<i', e, 0x10, r.choice([0, 1, 8, 16, 25, r.randrange(0, 26)]))  # bits_left
    struct.pack_into('<i', e, 0x24, r.choice([0, 0, 1, r.randrange(0, 50)]))  # restarts_to_go
    return e


def derived_tbl(r):
    regions = common(r)
    regions[ENTROPY] = entropy_bytes(r)
    dc = r.random() < 0.5
    tblno = r.choice([0, 1, 2, 3]) if r.random() < 0.96 else r.choice([-1, 4])  # (out of range: the error path, reading the neighbour)
    tables = bytearray()
    ptrs = []
    for i in range(4):
        tables += htbl_bytes(r, dc)[0]
        ptrs.append(0 if r.random() < 0.02 else HTBL + HTBL_SIZE * i)
    cinfo = regions[CINFO]
    struct.pack_into('<I', cinfo, DC_TBLS - 4, HTBL)
    struct.pack_into('<I', cinfo, AC_TBLS + 0x10, HTBL + HTBL_SIZE)
    struct.pack_into('<4I', cinfo, DC_TBLS, *ptrs)
    struct.pack_into('<4I', cinfo, AC_TBLS, *ptrs)
    have = r.random() < 0.3
    regions.update({HTBL: tables, PDTBL: struct.pack('<I', DTBL if have else 0), DTBL: r.bytes(DTBL_SIZE), HEAP: heap(0x600)})
    if not have:
        del regions[DTBL]
    return Call(0x6441f0, [CINFO, int(dc) | (r.randrange(256) << 8 if r.wild else 0), tblno, PDTBL], regions)


def stream(r, n=0x30):
    out = bytearray()
    while len(out) < n:
        k = r.random()
        if k < 0.6:
            out.append(r.randrange(0, 0xff))
        elif k < 0.8:
            out += b'\xff\x00'
        elif k < 0.9:
            out += b'\xff' * r.randrange(1, 4) + bytes([r.randrange(1, 0xff)])
        else:
            out += b'\xff'
    return bytes(out[:n])


def state_bytes(r, bits_left, get_buffer=None):
    n = r.choice([0, 1, 2, 4, r.randrange(0, 0x30)])
    gb = r.randrange(1 << 32) if get_buffer is None else get_buffer
    return struct.pack('<IIIiI', BUF + r.randrange(0, 0x30 - n + 1) if n < 0x30 else BUF, n, gb, bits_left, CINFO)


def source(r, regions):  # (MORE never starts with 0xFF: a refill of FF bytes only would loop for ever, in both)
    more = stream(r, MORE_SIZE)
    regions.update({BUF: stream(r), MORE: bytes([r.randrange(0, 0xff)]) + more[1:],
                    SRC: struct.pack('<II', MORE, r.choice([1, 2, 8, 0x30, MORE_SIZE])) + r.bytes(4) +
                    struct.pack('<I', recorder(K_FILL, 1, cdecl=True)) + r.bytes(0x10)})


def fill(r):
    regions = common(r)
    regions[ENTROPY] = entropy_bytes(r)
    source(r, regions)
    bits_left = r.choice([0, 1, 7, 8, 15, 16, 24, r.randrange(0, 25)])
    st = state_bytes(r, bits_left)
    regions[STATE] = st
    get_buffer = struct.unpack_from('<I', st, 8)[0]
    return Call(0x6444d0, [STATE, get_buffer, bits_left, r.choice([0, 1, 8, 16, 25, r.randrange(0, 26)])], regions, ret='b')


def decode(r):
    regions = common(r)
    regions[ENTROPY] = entropy_bytes(r)
    source(r, regions)
    raw, bits, vals = htbl_bytes(r, False, False)
    bits_left = r.choice([0, 1, 8, 16, 24, r.randrange(0, 25)])
    get_buffer = r.randrange(1 << 32)
    if r.random() < 0.5:  # (one of the table's codes next: the longest, more often than not)
        codes, sizes = codes_of(bits)
        p = len(codes) - 1 - (0 if r.random() < 0.6 else r.randrange(len(codes)))
        code, l = codes[p], sizes[p]
        bits_left = r.randrange(l, 25)
        shift = bits_left - l
        get_buffer = (get_buffer & ~(((1 << l) - 1) << shift) | code << shift) & 0xffffffff
    st = state_bytes(r, bits_left, get_buffer)
    regions.update({STATE: st, HTBL: raw, DTBL: derived(bits, HTBL, r=r)})
    min_bits = r.choice([1, 2, 8, r.randrange(1, 17)])
    return Call(0x6445e0, [STATE, get_buffer, bits_left, DTBL, min_bits], regions, ret='a')


def jinit(r):
    regions = common(r)
    regions[HEAP] = heap(0x200)
    return Call(0x6446d0, [CINFO], regions)


def scan(r, regions):
    """comps_in_scan components (dc / ac table numbers 0..3, component_needed, DCT_scaled_size) and blocks_in_MCU blocks of them."""
    cinfo = regions[CINFO]
    ncomp = r.choice([1, 1, 3, r.randrange(1, 5)])
    comps = bytearray()
    for ci in range(ncomp):
        c = bytearray(r.bytes(COMP_SIZE))
        struct.pack_into('<ii', c, 0x14, r.randrange(4), r.randrange(4))
        struct.pack_into('<i', c, 0x24, r.choice([1, 2, 4, 8]))
        c[0x30] = r.choice([1, 1, 1, 0])
        comps += c
    nblocks = r.choice([1, ncomp, r.randrange(1, 11)])
    member = [r.randrange(ncomp) for _ in range(nblocks)]
    struct.pack_into('<i', cinfo, COMPS_IN_SCAN, ncomp)
    struct.pack_into('<4I', cinfo, CUR_COMP_INFO, *[COMP + COMP_SIZE * i if i < ncomp else r.getrandbits(32) for i in range(4)])
    struct.pack_into('<i', cinfo, BLOCKS_IN_MCU, nblocks)
    struct.pack_into('<10i', cinfo, MCU_MEMBERSHIP, *(member + [r.randrange(4) for _ in range(10 - nblocks)]))
    regions[COMP] = comps
    return nblocks, member


def start(r):
    regions = common(r)
    cinfo = regions[CINFO]
    if r.random() < 0.85:
        struct.pack_into('<iiii', cinfo, SS, 0, 63, 0, 0)
    else:
        struct.pack_into('<iiii', cinfo, SS, r.choice([0, 1]), r.choice([0, 63]), r.choice([0, 1]), r.choice([0, 1]))
    scan(r, regions)
    tables = bytearray()
    for i in range(8):
        tables += htbl_bytes(r, i < 4)[0]
    struct.pack_into('<4I', cinfo, DC_TBLS, *[HTBL + HTBL_SIZE * i for i in range(4)])
    struct.pack_into('<4I', cinfo, AC_TBLS, *[HTBL + HTBL_SIZE * (4 + i) for i in range(4)])
    e = entropy_bytes(r)
    struct.pack_into('<8I', e, 0x28, *[0 if r.random() < 0.7 else (DTBL if i == 0 else DTBL2) for i in range(8)])
    regions.update({ENTROPY: e, HTBL: tables, DTBL: r.bytes(DTBL_SIZE), DTBL2: r.bytes(DTBL_SIZE), HEAP: heap(0x2c00)})
    return Call(0x644710, [CINFO], regions)


def mcu_tables(r, regions):
    """A DC (symbols 0..15) and an AC table, both with their full derived tables, at HTBL / HTBL + HTBL_SIZE and DTBL / DTBL2."""
    raw_dc, bits_dc, vals_dc = htbl_bytes(r, True, False, True)
    raw_ac, bits_ac, vals_ac = htbl_bytes(r, False, False)
    regions.update({HTBL: raw_dc + raw_ac, DTBL: derived(bits_dc, HTBL, vals_dc), DTBL2: derived(bits_ac, HTBL + HTBL_SIZE, vals_ac)})


def mcu(r):
    regions = common(r)
    source(r, regions)
    nblocks, member = scan(r, regions)
    cinfo = regions[CINFO]
    struct.pack_into('<I', cinfo, RESTART_INTERVAL, r.choice([0, 0, 1, r.randrange(1, 100)]))
    mcu_tables(r, regions)
    e = entropy_bytes(r)
    struct.pack_into('<10I', e, 0x48, *[DTBL] * 10)
    struct.pack_into('<10I', e, 0x70, *[DTBL2] * 10)
    for b in range(10):
        e[0x98 + b] = r.choice([1, 1, 1, 0])
        e[0xa2 + b] = r.choice([1, 1, 0])
    regions.update({ENTROPY: e, MCUDATA: struct.pack('<10I', *[BLOCKS + 0x80 * b for b in range(10)]), BLOCKS: r.bytes(0x80 * 10)})
    return Call(0x644860, [CINFO, MCUDATA], regions, ret='b')


def restart(r):
    regions = common(r)
    regions[ENTROPY] = entropy_bytes(r)
    struct.pack_into('<i', regions[ENTROPY], 0x10, r.choice([0, 7, 8, 9, 31, -1, -8, -9, r.randrange(-40, 40)]))
    struct.pack_into('<i', regions[CINFO], COMPS_IN_SCAN, r.choice([0, 1, 3, 4]))
    return Call(0x644cb0, [CINFO], regions, ret='b')


derived_tbl.__name__, fill.__name__, decode.__name__ = 'derived', 'fill', 'decode'
OPS = [f for f in (derived_tbl, fill, decode, jinit, start, mcu, restart)
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
