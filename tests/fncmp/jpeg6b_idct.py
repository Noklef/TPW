# libjpeg 6b's integer inverse DCTs (jidctint.c, jidctfst.c, jidctred.c; src/game/lib/jpeg6b: the library built from
# third_party/jpeg-6b) against the exe's copies. A coefficient block at COEF; the component at COMP with its dequantization table
# (compptr->dct_table, +0x50) at QUANT; cinfo at CINFO with sample_range_limit (+0x120) pointing into a random range-limit table at
# RANGE (the IDCTs index it from +CENTERJSAMPLE with RANGE_MASK, so any index they compute reads a byte both sides have); the output
# rows an array of row pointers at ROWS into OUTPUT, at output_col.
# jidctflt.c's jpeg_idct_float (0x64b040) stays the exe's: its (INT32) casts are __ftol in the exe, the low half of a 64-bit FISTP,
# and cvttss2si in the library (__ftol2_sse when built as x87 code), which differ past +-2^31 (16-bit quantizers reach it); the game
# never selects JDCT_FLOAT (its codec leaves dct_method, +0x44, at default_decompress_parms' JDCT_ISLOW).
# usage: jpeg6b_idct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=islow,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {
    'jpeg_idct_islow': (0x64a5c0, 0x64abb0), 'jpeg_idct_ifast': (0x64abb0, 0x64b040), 'jpeg_idct_4x4': (0x64b5b0, 0x64b970), 'jpeg_idct_2x2': (0x64b970, 0x64bb90), 'jpeg_idct_1x1': (0x64bb90, 0x64bbd0),
}
B = Bench(sorted(FUNCS.values()))

CINFO, COMP, COEF, QUANT, ROWS, RANGE, OUTPUT = RAM, RAM + 0x200, RAM + 0x300, RAM + 0x400, RAM + 0x600, RAM + 0x1000, RAM + 0x2000
SAMPLE_RANGE_LIMIT, DCT_TABLE = 0x120, 0x50
RANGE_SIZE = 5 * 256 + 128  # (jdmaster.c's prepare_range_limit_table; sample_range_limit is 256 bytes in)
# jidctfst.c's: (q * aanscales[i] + 2048) >> 12 (jddctmgr.c, IFAST_SCALE_BITS 2)
AANSCALES = [16384, 22725, 21407, 19266, 16384, 12873, 8867, 4520, 22725, 31521, 29692, 26722, 22725, 17855, 12299, 6270,
             21407, 29692, 27969, 25172, 21407, 16819, 11585, 5906, 19266, 26722, 25172, 22654, 19266, 15137, 10426, 5315,
             16384, 22725, 21407, 19266, 16384, 12873, 8867, 4520, 12873, 17855, 16819, 15137, 12873, 10114, 6967, 3552,
             8867, 12299, 11585, 10426, 8867, 6967, 4799, 2446, 4520, 6270, 5906, 5315, 4520, 3552, 2446, 1247]


def coefficients(r):
    """A block as the entropy decoder leaves one: mostly zeros past the first few in zigzag order (so whole columns or rows of AC
    terms are zero, the IDCTs' shortcut), sometimes dense; values a baseline stream gives (11 bits), extremes when wild."""
    big = 32767 if r.wild and r.random() < 0.3 else 2047
    shape = r.random()
    c = [0] * 64
    if shape < 0.3:  # (DC only, or DC and a few low terms)
        c[0] = r.randrange(-big, big + 1)
        for _ in range(r.choice([0, 0, 1, 2, 3])):
            c[r.choice([1, 2, 8, 9, 16, 3, 24])] = r.randrange(-64, 65)
    elif shape < 0.6:  # (one column or one row of AC terms only)
        col = r.randrange(8)
        for i in r.sample(range(8), r.randrange(1, 9)):
            c[(i * 8 + col) if r.random() < 0.5 else (col * 8 + i)] = r.randrange(-big, big + 1)
    elif shape < 0.8:  # (sparse)
        for i in r.sample(range(64), r.randrange(1, 12)):
            c[i] = r.choice([r.randrange(-8, 9), r.randrange(-big, big + 1)])
    else:  # (dense)
        c = [r.randrange(-big, big + 1) if r.random() < 0.7 else 0 for _ in range(64)]
    return struct.pack('<64h', *c)


def quant_ints(r, fast):
    q = [r.choice([1, 1, r.randrange(1, 32), r.randrange(1, 256)]) for _ in range(64)] if r.random() < 0.5 else \
        [r.randrange(1, 100)] * 64
    if r.wild and r.random() < 0.3:
        q = [r.randrange(0, 0x8000) for _ in range(64)]
    if fast:
        q = [(v * AANSCALES[i] + 2048) >> 12 for i, v in enumerate(q)]
    return struct.pack('<64i', *q)


def idct(address, size, fast=False):
    def op(r):
        cinfo = bytearray(r.bytes(0x1b0))
        struct.pack_into('<I', cinfo, SAMPLE_RANGE_LIMIT, RANGE + 256)
        comp = bytearray(r.bytes(0x54))
        struct.pack_into('<I', comp, DCT_TABLE, QUANT)
        col = r.choice([0, 0, size, r.randrange(64)])
        stride = col + size + r.randrange(4)
        order = list(range(size))
        if r.random() < 0.3:
            r.shuffle(order)
        rows = struct.pack('<%dI' % size, *(OUTPUT + i * stride for i in order))
        regions = {CINFO: cinfo, COMP: comp, COEF: coefficients(r), QUANT: quant_ints(r, fast),
                   ROWS: rows, RANGE: r.bytes(RANGE_SIZE), OUTPUT: r.bytes(size * stride)}
        return Call(address, [CINFO, COMP, COEF, ROWS, col], regions)
    op.__name__ = {0x64a5c0: 'islow', 0x64abb0: 'ifast', 0x64b5b0: 'idct4x4', 0x64b970: 'idct2x2', 0x64bb90: 'idct1x1'}[address]
    return op


OPS = [f for f in (idct(0x64a5c0, 8), idct(0x64abb0, 8, fast=True), idct(0x64b5b0, 4), idct(0x64b970, 2), idct(0x64bb90, 1))
       if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
