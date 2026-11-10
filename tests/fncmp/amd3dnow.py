# The 3DNow! matrix routines (src/game/math/amd3dnow: 0x5f4730, 0x5f4844, 0x5f4bd0) against the originals, run by tools/x87emu.py's 3DNow! (PFADD, PFMUL, PFACC as IEEE singles).
# The floats are normal with exponents near 0, and zeros, so that every product and sum stays normal: there 3DNow! and IEEE agree, and its departures (denormals,
# infinities, NaNs, overflow) are left alone. Sources and destinations any alignment (MOVQ asks none); destinations on the matrix and on the source; strides small,
# 0 and overlapping; the batch's and the vectors' count 0 (2^32 runs, a fault in both) now and then.
# usage: amd3dnow.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Batch,Vectors,Multiply)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
import x87emu  # noqa: E402
from fncmp import RAM, Bench, Call  # noqa: E402


def _pair(a, b):
    return int.from_bytes(struct.pack('<2f', a, b), 'little')


# x87emu's 3DNow! against sums worked by hand.
assert x87emu.MMX['PFACC'](_pair(1.0, 2.0), _pair(3.0, 4.5)) == _pair(3.0, 7.5)
assert x87emu.MMX['PFMUL'](_pair(1.5, -2.0), _pair(3.0, 0.25)) == _pair(4.5, -0.5)
assert x87emu.MMX['PFADD'](_pair(1.0, 2.0 ** 24), _pair(2.0 ** -30, 1.0)) == _pair(1.0, 2.0 ** 24)   # (rounded to a single, ties to even)

B = Bench([(0x5f4730, 0x5f4844), (0x5f4844, 0x5f4a46), (0x5f4bd0, 0x5f4e98)])
OPS = [k for k in os.environ.get('OPS', 'Batch,Vectors,Multiply').split(',') if k]
MATRIX, SOURCE, DEST = RAM, RAM + 0x100, RAM + 0x1000


def number(r):
    k = r.randrange(16)
    if k == 0:
        return r.choice([0, 0x80000000])
    if k == 1:
        return struct.unpack('<I', struct.pack('<f', r.choice([1.0, -1.0, 2.0, 0.5, 3.0])))[0]
    return r.getrandbits(1) << 31 | r.randrange(127 - 20, 127 + 20) << 23 | r.getrandbits(23)


def floats(r, count):
    return b''.join(struct.pack('<I', number(r)) for _ in range(count))


def case(r):
    op = r.deal(OPS)
    regions = {MATRIX: floats(r, 0x40), SOURCE: floats(r, 0x300), DEST: floats(r, 0x300)}
    this = r.getrandbits(32)
    matrix = MATRIX + r.choice([0, 0, 4, 8, 0x10])
    forever = not r.randrange(40)   # (count 0: on until the destination leaves its chunk)
    if op == 'Batch':
        count = r.choice([1, 2, 3, 4, 5, 7, 8, 9]) if not forever else r.choice([0, 0xffffffff])
        dest = r.choice([DEST, DEST, DEST + 4, DEST + 8, MATRIX, SOURCE + 0x30])
        return Call(0x5f4730, [this, matrix, dest, SOURCE + r.choice([0, 4, 0x10]), r.choice([0x10, 0x10, 0x20, 8, 0xc, 0]) if not forever else 0x10, r.randrange(12), count],
                    regions, ret='v')
    if op == 'Vectors':
        mode = r.deal([1, 2, 3, 4, 0, 8])
        if mode == 8:
            mode = r.getrandbits(32) | 8
        source = SOURCE + r.choice([0, 4, 8, 0xc])
        dest = r.choice([DEST, DEST + 4, source, source, MATRIX])
        count = r.randrange(1, 7) if not forever else 0
        return Call(0x5f4844, [this, matrix, dest, source, r.choice([0x10, 0xc, 8, 0x14, 0x20, 0]) if not forever else 0x10, r.choice([0xc, 0x10, 0x14, 0x20, 0]), count, mode],
                    regions, ret='v')
    a, b = SOURCE + r.choice([0, 4, 0x40]), SOURCE + 0x100 + r.choice([0, 8])
    return Call(0x5f4bd0, [this, a, b, r.choice([DEST, DEST + 4, a, b, a + 8, a + 0x10, b + 4, b + 0x20])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
