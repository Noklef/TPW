# libjpeg 6b's jdhuff.c, the sequential Huffman decoder (src/game/lib/jpeg6b: the library built from third_party/jpeg-6b) against the
# exe's copies. cinfo at CINFO (err +0 at ERR: error_exit a cdecl recorder (slot 0), emit_message +4 another (slot 1, (cinfo, level));
# msg_code +0x14, msg_parm +0x18; mem +4 at MEM: alloc_small ZALLOC over the case heap; src +0x18 at SRC: next_input_byte,
# bytes_in_buffer, fill_input_buffer +0xc a cdecl recorder (slot 2) whose answer is a hash (TRUE) or set to 0 (FALSE): the source is
# the game's own (see docs/porting.md), so a "refill" here hands back what src points at, a second buffer at MORE; the DC / AC tables
# dc_huff_tbl_ptrs[4] +0xa0, ac +0xb0 at HTBL; unread_marker +0x17c; entropy +0x198 at ENTROPY, insufficient_data +8, a byte).
# jpeg_make_d_derived_tbl builds a d_derived_tbl (0x590 bytes: maxcode[18], valoffset[17], pub +0x8c, look_nbits[256] +0x90,
# look_sym[256] +0x490) from a JHUFF_TBL (bits[17], huffval[256]); the tables here are valid Huffman codes (the code space left
# free), sometimes with symbols over 15 for a DC table or a code that does not fit (both error paths, which go on). Its locals
# (huffsize[257], huffcode[257]) are written before they are read for any table whose counts sum to 256 or less, which all of these
# do (a larger sum overruns them in both, past what a bench can compare). jpeg_fill_bit_buffer and jpeg_huff_decode take a
# bitread_working_state at STATE (next_input_byte, bytes_in_buffer, get_buffer, bits_left, cinfo) and the bytes at BUF: 0xFF 0x00
# stuffing, markers (0xFF then another byte: unread_marker, then zeros), and FF FF runs. jpeg_huff_decode's table is a
# d_derived_tbl computed here as jpeg_make_d_derived_tbl makes it (its maxcode[17] the 0xFFFFF sentinel).
# usage: jpeg6b_huff.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=derived,fill,decode)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, LOG, RAM, ZALLOC, Bench, Call, heap, log_region, recorder, set_return  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_make_d_derived_tbl': (0x6441f0, 0x6444d0), 'jpeg_fill_bit_buffer': (0x6444d0, 0x6445e0),
         'jpeg_huff_decode': (0x6445e0, 0x6446d0)}
B = Bench(sorted(FUNCS.values()))

CINFO, ERR, MEM, SRC, ENTROPY, STATE, PDTBL, HTBL, DTBL, BUF, MORE = (RAM, RAM + 0x200, RAM + 0x280, RAM + 0x300, RAM + 0x340,
                                                                       RAM + 0x360, RAM + 0x380, RAM + 0x400, RAM + 0x1000,
                                                                       RAM + 0x2000, RAM + 0x2100)
SRC_, DC_TBLS, AC_TBLS, UNREAD_MARKER, ENTROPY_ = 0x18, 0xa0, 0xb0, 0x17c, 0x198
K_ERROR, K_EMIT, K_FILL = 0, 1, 2
HTBL_SIZE = 0x114  # JHUFF_TBL: bits[17], huffval[256], sent_table (and padding)


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


def htbl_bytes(r, dc, overfull=True):
    bits = huff_bits(r, overfull)
    n = sum(bits)
    vals = [r.randrange(0, 16) if dc and r.random() < 0.95 else r.randrange(256) for _ in range(n)]
    return bytes(bits) + bytes(vals) + r.bytes(256 - n) + r.bytes(HTBL_SIZE - 17 - 256), bits, vals


def derived(bits, htbl_address):
    """jpeg_make_d_derived_tbl's maxcode and valoffset (the lookahead tables are left random: jpeg_huff_decode does not read them)."""
    sizes = [l for l in range(1, 17) for _ in range(bits[l])]
    codes, code, si, p = [], 0, sizes[0] if sizes else 0, 0
    while p < len(sizes):
        while p < len(sizes) and sizes[p] == si:
            codes.append(code)
            code += 1
            p += 1
        code <<= 1
        si += 1
    maxcode, valoffset, p = [0] * 18, [0] * 17, 0
    for l in range(1, 17):
        if bits[l]:
            valoffset[l] = p - codes[p]
            p += bits[l]
            maxcode[l] = codes[p - 1]
        else:
            maxcode[l] = -1
    maxcode[17] = 0xfffff
    return struct.pack('<18i', *maxcode) + struct.pack('<17i', *valoffset) + struct.pack('<I', htbl_address)


def cinfo_bytes(r):
    cinfo = bytearray(r.bytes(0x1b0))
    struct.pack_into('<II', cinfo, 0, ERR, MEM)
    struct.pack_into('<I', cinfo, SRC_, SRC)
    struct.pack_into('<I', cinfo, ENTROPY_, ENTROPY)
    struct.pack_into('<i', cinfo, UNREAD_MARKER, 0 if r.random() < 0.8 else r.randrange(1, 256))
    return cinfo


def common(r):
    log = log_region()
    if r.random() < 0.15:
        set_return(log, K_FILL, 0)  # (fill_input_buffer suspends)
    return {CINFO: cinfo_bytes(r), ERR: struct.pack('<II', recorder(K_ERROR, 1, cdecl=True), recorder(K_EMIT, 2, cdecl=True)) + r.bytes(0x78),
            MEM: struct.pack('<I', ZALLOC) + r.bytes(0x2c), LOG: log,
            ENTROPY: r.bytes(8) + bytes([r.choice([0, 0, 1])]) + r.bytes(7)}


def derived_tbl(r):
    regions = common(r)
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
    regions.update({HTBL: tables, PDTBL: struct.pack('<I', DTBL if have else 0), DTBL: r.bytes(0x590), HEAP: heap(0x600)})
    if not have:
        del regions[DTBL]
    return Call(0x6441f0, [CINFO, int(dc) | (r.randrange(256) << 8 if r.wild else 0), tblno, PDTBL], regions)


def stream(r):
    out = bytearray()
    while len(out) < 0x30:
        k = r.random()
        if k < 0.6:
            out.append(r.randrange(0, 0xff))
        elif k < 0.8:
            out += b'\xff\x00'
        elif k < 0.9:
            out += b'\xff' * r.randrange(1, 4) + bytes([r.randrange(1, 0xff)])
        else:
            out += b'\xff'
    return bytes(out[:0x30])


def state_bytes(r, bits_left):
    n = r.choice([0, 1, 2, 4, r.randrange(0, 0x30)])
    return struct.pack('<IIIiI', BUF + r.randrange(0, 0x30 - n + 1) if n < 0x30 else BUF, n, r.randrange(1 << 32), bits_left, CINFO)


def source(r, regions):  # (MORE never starts with 0xFF: a refill of FF bytes only would loop for ever, in both)
    regions.update({BUF: stream(r), MORE: bytes([r.randrange(0, 0xff)]) + stream(r)[1:], SRC: struct.pack('<II', MORE, r.choice([1, 2, 8, 0x30])) + r.bytes(4) +
                    struct.pack('<I', recorder(K_FILL, 1, cdecl=True)) + r.bytes(0x10)})


def fill(r):
    regions = common(r)
    source(r, regions)
    bits_left = r.choice([0, 1, 7, 8, 15, 16, 24, r.randrange(0, 25)])
    st = state_bytes(r, bits_left)
    regions[STATE] = st
    get_buffer = struct.unpack_from('<I', st, 8)[0]
    return Call(0x6444d0, [STATE, get_buffer, bits_left, r.choice([0, 1, 8, 16, 25, r.randrange(0, 26)])], regions, ret='b')


def decode(r):
    regions = common(r)
    source(r, regions)
    raw, bits, vals = htbl_bytes(r, False, False)
    while not sum(bits):
        raw, bits, vals = htbl_bytes(r, False, False)
    bits_left = r.choice([0, 1, 8, 16, 24, r.randrange(0, 25)])
    st = state_bytes(r, bits_left)
    regions.update({STATE: st, HTBL: raw, DTBL: derived(bits, HTBL) + r.bytes(0x590 - 0x90)})
    get_buffer = struct.unpack_from('<I', st, 8)[0]
    return Call(0x6445e0, [STATE, get_buffer, bits_left, DTBL, r.choice([1, 2, 8, r.randrange(1, 17)])], regions, ret='a')


derived_tbl.__name__, fill.__name__, decode.__name__ = 'derived', 'fill', 'decode'
OPS = [f for f in (derived_tbl, fill, decode) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
