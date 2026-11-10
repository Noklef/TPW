# libjpeg 6a's integer forward DCTs (src/game/lib/jpeg6a: the library built from third_party/jpeg-6a, its names jpeg6a_*) against the
# exe's 6a copies: jpeg_fdct_islow (jfdctint.c, 0x657810) and jpeg_fdct_ifast (jfdctfst.c, 0x657480), each one block of 64 DCTELEMs
# (ints) in place at DATA. jcdctmgr.c's forward_DCT hands them samples less CENTERJSAMPLE (-128..127); the cases are those, flat and
# edge blocks, and wider values (the wild ones the whole int range: both wrap the same).
# jpeg_fdct_float (jfdctflt.c, 0x657110) is a hand port (src/game/lib/jpegfloatdct: Jpeg6a_FdctFloat, x87 in the listing's order):
# 64 floats in place, the same blocks as floats, and when wild random float bits among them (NaNs, infinities, sums past the float
# range).
# usage: jpeg6a_fdct.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=islow,ifast,fdct_float)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

# the exe's copies (re/libmap.csv): name -> (address, end)
FUNCS = {'jpeg_fdct_ifast': (0x657480, 0x657802), 'jpeg_fdct_islow': (0x657810, 0x657cb3), 'jpeg_fdct_float': (0x657110, 0x657474)}
B = Bench(sorted(FUNCS.values()))

DATA = RAM


def block(r):
    kind = r.choice(['samples', 'samples', 'samples', 'flat', 'edge', 'sparse', 'wide'])
    if kind == 'flat':
        v = r.choice([-128, 127, 0, r.randrange(-128, 128)])
        vals = [v] * 64
    elif kind == 'edge':
        lo, hi = r.choice([(-128, 127), (127, -128), (r.randrange(-128, 128), r.randrange(-128, 128))])
        cut = r.randrange(1, 8)
        vals = [lo if (i % 8 if r.random() < 0.5 else i // 8) < cut else hi for i in range(64)]
    elif kind == 'sparse':
        vals = [r.randrange(-128, 128) if r.random() < 0.1 else 0 for _ in range(64)]
    elif kind == 'wide':
        bits = r.choice([12, 16, 20, 24])
        vals = [r.randrange(-1 << bits, 1 << bits) for _ in range(64)]
    else:
        vals = [r.randrange(-128, 128) for _ in range(64)]
    if r.wild:
        vals = [r.randrange(-1 << 31, 1 << 31) if r.random() < 0.3 else v for v in vals]
    return struct.pack('<64i', *vals)


def islow(r):
    return Call(0x657810, [DATA], {DATA: block(r)})


def ifast(r):
    return Call(0x657480, [DATA], {DATA: block(r)})


def fdct_float(r):
    vals = struct.unpack('<64i', block(r))
    bits = [fb(float(v)) for v in vals]
    if r.random() < 0.3:
        bits = [fb(r.uniform(-1, 1) * r.choice([1, 128, 1e6, 1e30, 3e38])) for _ in range(64)]
    if r.wild:
        bits = [r.f() if r.random() < 0.3 else b for b in bits]
    return Call(0x657110, [DATA], {DATA: struct.pack('<64I', *bits)})


OPS = [f for f in (islow, ifast, fdct_float) if not os.environ.get('OPS') or f.__name__ in os.environ['OPS'].split(',')]


def case(r):
    return r.deal(OPS)(r)


if __name__ == '__main__':
    B.main(case)
