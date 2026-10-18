# A footprint's cells and ground height (src/game/terrain/land: Terrain_FootprintCell 0x454050, Terrain_FootprintHeight 0x466d70) against
# the originals, dealt in turn.
# FootprintCell: the four right angles (and now and then another), corners whole or random floats (sometimes NaN or huge), fractions in
# 0..1 (sometimes the edges, odd or wild).
# FootprintHeight: a map (0x7a0854) whose grid (+0x6c) is 24 cells wide with heights about -50..50 (now and then infinite or NaN); a
# footprint 1-6 by 1-6 (sometimes 0 or negative) at a cell 0-10, turned by a right angle (now and then another, at cell (0, 0): the corner
# read at the floats' bits), with or without its cells (dwords of random low bits) and flag 0x10.
# usage: footprint.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x454050, 0x4541a0), (0x466d70, 0x466fa0), (0x469930, 0x469950)])
OUT, MAP, GRID, HEIGHTS, CELLS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
ANGLES = [0, 90, 180, 270]
GW = 24


def corner(r):
    if r.random() < 0.1:
        return r.f()
    return fb(float(r.randrange(-5, 40)))


def fraction(r):
    p = r.random()
    if p < 0.1:
        return r.f()
    if p < 0.3:
        return fb(r.choice([0.0, 1.0, 0.5, 0.25, 1.0 / 3]))
    return fb(r.random())


def cell_case(r):
    angle = r.choice(ANGLES * 6 + [r.randrange(-400, 400)])
    args = [angle] + [corner(r) for _ in range(4)] + [fraction(r), fraction(r), OUT, OUT + 4]
    return Call(0x454050, args, {OUT: r.bytes(8)})


def height(r):
    if r.random() < 0.03:
        return r.choice([float('nan'), float('inf'), float('-inf'), 3e38])
    return float(r.randrange(-50, 51)) if r.random() < 0.3 else r.uniform(-50, 50)


def height_case(r):
    grid = put(bytearray(r.bytes(0x30)), 0x18, 'I', GW)
    put(grid, 0x28, 'I', HEIGHTS)
    heights = bytearray(4 * (GW + 1) * 20)
    for i in range(len(heights) // 4):
        put(heights, 4 * i, 'f', height(r))
    w, d = r.choice([1, 2, 3, 4, 6, r.randrange(1, 7)]), r.choice([1, 2, 3, 5, r.randrange(1, 7)])
    if r.random() < 0.05:
        w, d = r.choice([(0, d), (w, 0), (-1, d), (w, -2)])
    angle = r.choice(ANGLES * 10 + [45])
    cells = bytearray(8 + 8 * 6 + 0xa0 * 6)
    for i in range(len(cells) // 4):
        put(cells, 4 * i, 'I', r.getrandbits(32) & r.choice([0, 0xf0, 1, 2, 4, 8, 0xffffffff]))
    regions = {0x7a0854: struct.pack('<I', MAP), MAP: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID), GRID: grid, HEIGHTS: heights,
               CELLS: cells}
    flags = r.getrandbits(32) & ~0x10 | r.choice([0, 0x10])
    x, y = r.randrange(0, 11), r.randrange(0, 11)
    if angle == 45:   # (the corner is then read at the floats' bits: 0.0's are cell (0, 0); others land anywhere, the driver's image too)
        x = y = 0
    return Call(0x466d70, [x, y, r.choice([CELLS, CELLS, 0]), w, d, flags, angle], regions, ret='d')


def case(r):
    return r.deal([cell_case, height_case])(r)


if __name__ == '__main__':
    B.main(case)
