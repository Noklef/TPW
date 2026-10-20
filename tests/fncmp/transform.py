# The vertex array transforms (src/game/math/transform: 0x579250, 0x579330, 0x579690, 0x579800) against the originals: any matrices
# and vertices (special values now and then, and NaNs tying on significand), 0-8 vertices, strides forwards, backwards, zero and
# overlapping, the output sometimes the input itself or just past it, every mode (and others), a first vertex past the second block
# for the blocked one, any render view (its depth threshold's bits sometimes one of the z's) for the projection, and products into
# either factor or across them; and the SSE probe (0x579a80: its __try frame on FS:[0], made and taken down).
# usage: transform.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Blocked,Vertices,Project,Concat,Probe)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x579250, 0x579a50), (0x579a80, 0x579af9), (0x5f4ea0, 0x5f4eab)])
OPS = [k for k in os.environ.get('OPS', 'Blocked,Vertices,Project,Concat,Probe').split(',') if k]
MATRIX, BUF, SIZE = RAM, RAM + 0x100, 0x600
MID = BUF + 0x300
VIEW = 0x8bcbc8
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
    if op == 'Probe':
        return Call(0x579a80, [], regions, ret='a')
    if op == 'Concat':
        a, b = MID, MID + 0x40
        out = r.choice([MID + 0x100, a, b, b, MID + 0x20, MID - 0x20])
        return Call(0x579800, [r.getrandbits(32), a, b, out], regions)
    if op == 'Project':
        view = r.floats(bytearray(0x78), range(0, 0x78, 4), r.choice([1, 300, None]))
        if r.random() < 0.3:
            put(view, 0x60, 'I', struct.unpack_from('<I', regions[BUF], MID + 8 - BUF)[0])
        if r.random() < 0.1:
            put(view, 0x74, 'I', r.choice(NANS))
        if r.random() < 0.25:  # NaN z's, some tying on significand, so the order of the sum's operands shows
            for k in range(count):
                if r.random() < 0.5:
                    put(regions[BUF], MID - BUF + 8 + k * out_stride, 'I', r.choice(NANS))
        regions[VIEW] = view
        mode = r.choice([0, 1, 2, 0, 1, 2, 3, -1, r.getrandbits(32)])
        return Call(0x579690, [r.getrandbits(32), MID, out_stride, count, mode], regions, ret='d')
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
