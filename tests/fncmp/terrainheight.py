# The ground height (src/game/terrain/land: Terrain_HeightAt, 0x4527f0) against the original: a map (0x7a0854) whose grid (+0x6c) is
# 0-8 cells wide and high (now and then wide or random), its cell sizes mostly 10 (or 5, 2.5, 20, odd ones, 0, NaN), its heights about
# -50..50 (whole, fractional, now and then huge, infinite or NaN) and its cells' flags any mix of 0x800 and 4; the point anywhere on the
# grid, on cell borders and on the two diagonals (fx + fy = 1 and fx = fy, exact with size 10), at the edges, below 0 or past the far
# edge, -0.0, NaN or infinite.
# usage: terrainheight.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x4527f0, 0x452a70)])
MAP, GRID, HEIGHTS, CELLS = RAM, RAM + 0x100, RAM + 0x1000, RAM + 0x3000
ODD = [float('nan'), float('inf'), float('-inf'), 3e38, -3e38, 1e-40]


def height(r):
    if r.random() < 0.05:
        return r.choice(ODD)
    return float(r.randrange(-50, 51)) if r.random() < 0.3 else r.uniform(-50, 50)


def coordinate(r, n, size, other):
    """A coordinate on a grid n cells long of cells size long; other: the fraction of the other coordinate (for the diagonals)."""
    edge = n * 10.0
    pick = r.random()
    if pick < 0.08:
        return r.choice([0.0, -0.0, -1.0, -1e-6, edge, edge - 0.001, edge + 1, float('nan'), float('inf'), float('-inf'), 1e30])
    if n and size == 10.0 and pick < 0.45:   # (the cell's diagonals and borders, exact: x / 10 is whole plus the fraction)
        whole = r.randrange(n)
        frac = r.choice([other, 1.0 - other, 0.0, 0.5, 0.25, 0.75, other])
        return whole * 10.0 + frac * 10.0
    return r.uniform(0, max(edge, 1.0))


def case(r):
    w, h = (r.choice([0, 1, 2, 3, 4, 5, 8, 8]) for _ in range(2))
    if r.random() < 0.03:
        w, h = r.getrandbits(32), r.getrandbits(32)
    size_x, size_y = (r.choice([10.0] * 6 + [5.0, 2.5, 20.0, 7.3, r.uniform(1, 30), 0.0, float('nan')]) for _ in range(2))
    grid = bytearray(r.bytes(0x30))
    put(grid, 0x10, '2f', size_x, size_y)
    put(grid, 0x18, '2I', w, h)
    put(grid, 0x28, '2I', HEIGHTS, CELLS)
    heights = bytearray(0x2000)
    for i in range(0x800):
        put(heights, 4 * i, 'f', height(r))
    cells = bytearray(r.bytes(0x2000))
    for i in range(0x800):
        put(cells, 4 * i, 'H', r.getrandbits(16) & ~0x804 | r.choice([0, 4, 0x800, 0x804, 0x804]))
    fy = r.choice([0.0, 0.25, 0.5, 0.75])
    x = coordinate(r, w if w < 64 else 8, size_x, fy)
    y = coordinate(r, h if h < 64 else 8, size_y, (x / 10.0) % 1.0 if x == x and abs(x) < 1e9 else 0.0)
    regions = {0x7a0854: struct.pack('<I', MAP), MAP: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID), GRID: grid, HEIGHTS: heights, CELLS: cells}
    return Call(0x4527f0, [fb(x), fb(y)], regions, ret='d')


if __name__ == '__main__':
    B.main(case)
