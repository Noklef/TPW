# The wall follower's step (src/game/world/wallfollow: 0x511c90) against the original, with MapCell_StepBlocked and its callees run on
# cellstep.py's cells: a walker about the map's edges heading each way (0-3, past 3, 0x32 back on the line, 0x33 given up), turning either way
# (or by other amounts); its target and the cell where it left the line on, round or away from it; its count of cells recorded low, about 0xfb
# and past it (where the record lands on the direction and the turn); the mode (0x7cf3c0) with its upper bytes at times set.
# usage: wallfollow.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import cellstep  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x511c90, 0x511ec7), (0x4d8750, 0x4d8b29), (0x4d0af0, 0x4d0b06), (0x522700, 0x5228b5), (0x536300, 0x536484), (0x53ad20, 0x53aefc),
           (0x4dd4e0, 0x4dd510), (0x412e90, 0x412f00), (0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0), (0x44b220, 0x44b280),
           (0x549d30, 0x549dc0), (0x55a070, 0x55a0c0), (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0), (0x5da3c0, 0x5da3c1)])  # (cellstep.py's)
WALKER = RAM + 0x40000
STEP = bytes([0, 255, 1, 0, 0, 1, 255, 0, 109, 79, 118, 101, 114, 114, 105, 100])  # 0x7622b0, directions 0-7
STEP32 = bytes([110, 100])  # 0x762314, direction 0x32


def near(r, v):
    return r.choice([v, v, (v + 1) & 0xff, (v - 1) & 0xff, (v + r.randrange(-4, 5)) & 0xff, r.randrange(0x80), r.getrandbits(8)])


def case(r):
    mode = r.choice([0, 1, 1, 2, 2, 2, 3, -1]) & 0xffffffff
    if r.random() < 0.15:
        mode |= r.getrandbits(8) << 8
    d = r.choice([0, 1, 2, 3] * 4 + [0x32, 0x32, 0x33, 4, 5, 6, 7])
    turn = r.choice([1, 3, 1, 3, 0, 2, r.getrandbits(32)])
    x = r.choice([0, 1, 0x7e, 0x7f, 0x80, r.randrange(0x80), r.randrange(0x80)])
    y = r.choice([0, 1, 0x7e, 0x7f, r.randrange(0x80), r.randrange(0x80)])
    tx, ty, lx, ly = near(r, x), near(r, y), near(r, x), near(r, y)
    count = r.choice([0, 1, r.randrange(0xfc), r.randrange(0xfc), r.randrange(0xfc), 0xfa, 0xfa, 0xfb, r.choice([0xfc, 0xfd, 0xfe]), 0xff])
    sx, sy = (STEP32 if d == 0x32 else STEP[d * 2:d * 2 + 2]) if d != 0x33 else (0, 0)
    nx, ny = (x + sx) & 0xff, (y + sy) & 0xff
    steps = []
    for cx, cy, cd in [((mode & 0xffffff00) | x, y, d), (nx, ny, (d + turn) & 3)]:
        target, source = cellstep.target_of(cx, cy, cd)
        steps.append((source, target, cd & 0xff))
    regions = cellstep.world(r, mode - (1 << 32) if mode & 0x80000000 else mode, steps)
    walker = bytearray(r.bytes(0x20c))
    walker[0], walker[4], walker[5] = count, tx, ty
    put(walker, 0x200, 'II', d, turn & 0xffffffff)
    walker[0x208:0x20c] = bytes([x, y, lx, ly])
    regions[WALKER] = bytes(walker)
    regions[0x7cf3c0] = struct.pack('<I', mode)
    return Call(0x511c90, [WALKER], regions)


if __name__ == '__main__':
    B.main(case)
