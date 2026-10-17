# The vertex array transforms (src/game/math/transform: 0x579250, 0x579330) against the originals: any matrices and vertices (special
# values now and then, and NaNs tying on significand), 0-8 vertices, strides forwards, backwards, zero and overlapping, the output
# sometimes the input itself or just past it, every mode (and others), and a first vertex past the second block for the blocked one.
# usage: transform.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Blocked,Vertices)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x579250, 0x5796a0)])
OPS = [k for k in os.environ.get('OPS', 'Blocked,Vertices').split(',') if k]
MATRIX, BUF, SIZE = RAM, RAM + 0x100, 0x600
MID = BUF + 0x300
NANS = [0x7fc00000, 0xffc00000, 0x7fc00001, 0xffc00001, 0x7f800001, 0xff800001]


def case(r):
    op = r.deal(OPS)
    scale = r.choice([1, 10, 300, None])
    regions = {MATRIX: r.floats(bytearray(0x40), range(0, 0x40, 4), scale), BUF: r.floats(bytearray(SIZE), range(0, SIZE, 4), scale)}
    if r.random() < 0.1:  # NaNs that tie on significand, so which operand survives shows
        for buf in regions.values():
            for off in r.sample(range(0, len(buf), 4), len(buf) // 16):
                put(buf, off, 'I', r.choice(NANS))
    count = r.choice([0, 1, 1, 2, 3, 4, 5, 8])
    out_stride = r.choice([16, 16, 12, 20, 32, 0, -16, 4])
    if op == 'Blocked':
        src = MID - 0x80 + 4 * r.randrange(4)
        dst = r.choice([MID + 0x40, src, src + 4, MID - 0x100])
        first = r.choice([r.randrange(8), r.randrange(8), r.randrange(32), 0xfffffffc + r.randrange(4)])
        return Call(0x579250, [r.getrandbits(32), MATRIX, dst, src, out_stride, first, count], regions)
    mode = r.choice([0, 1, 2, 3, 0, 1, 2, 3, 4, -1, r.getrandbits(32)])
    in_stride = r.choice([16, 16, 12, 20, 32, 0, -16, 4])
    src = MID + 4 * r.randrange(-8, 8)
    dst = r.choice([src, src, MID + 0xc0, MID - 0xc0, src + 4, src - 8, src + 12])
    return Call(0x579330, [r.getrandbits(32), MATRIX, dst, src, out_stride, in_stride, count, mode], regions)


if __name__ == '__main__':
    B.main(case)
