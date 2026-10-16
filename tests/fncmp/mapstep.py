# The cell steps of src/game/world/mapcell (Map_StepCell 0x4d96f0, Map_StepCellInDirection 0x4d97e0) against the originals: cell ids over the map and its edges, offsets
# and scales either side of zero, and the eight direction pairs (small or large values) with other direction values.
# usage: mapstep.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Step,Dir)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Step': 0x4d96f0, 'Dir': 0x4d97e0}
B = Bench([(0x4d96f0, 0x4d97b0), (0x4d97b0, 0x4d97e0), (0x4d97e0, 0x4d9920), (0x4d99c0, 0x4d99e0), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1)])
CELL = RAM


def case(r):
    name = r.choice(OPS)
    cell = r.choice([1, 2, 0x7f, 0x80, 0x81, 0x4000, 0x3f81, 0x2000, r.randrange(1, 0x4001), 0, 0x4001, r.getrandbits(16)])
    regions = {CELL: struct.pack('<I', cell), 0x80239c: struct.pack('<I', RAM + 0x1000 + 4 * r.randrange(0x100))}
    small = lambda: r.choice([0, 1, -1, 2, -2, 3, 127, -127, 128, r.randrange(-5, 6), r.getrandbits(32) - (1 << 31)]) & 0xffffffff  # noqa: E731
    if name == 'Step':
        return Call(CALLS[name], [CELL, small(), small(), r.choice([1, 1, 0, 2, 3, -1 & 0xffffffff, small()])], regions, conv='t', ret='a')
    pairs = bytearray(0x40)
    for i in range(0, 0x40, 4):
        struct.pack_into('<I', pairs, i, r.choice([0, 1, 0xffffffff, 2, 0xfffffffe, 0x7f, r.getrandbits(8)]))
    regions[0x7cdba0] = pairs
    return Call(CALLS[name], [CELL, r.choice([1, 2, 4, 8, 0x10, 0x20, 0x40, 0x80, 0, 3, 0x100, 0x101, 0xff, r.getrandbits(32)])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
