# The ground height (src/game/terrain/land: Terrain_HeightAt, 0x4527f0) against the original: a map (0x7a0854) whose grid (+0x6c) is
# 0-8 cells wide and high (now and then wide or random), its cell sizes mostly 10 (or 5, 2.5, 20, odd ones, 0, NaN), its heights about
# -50..50 (whole, fractional, now and then huge, infinite or NaN) and its cells' flags any mix of 0x800 and 4; the point anywhere on the
# grid, on cell borders and on the two diagonals (fx + fy = 1 and fx = fy, exact with size 10), at the edges, below 0 or past the far
# edge, -0.0, NaN or infinite. Grid: Terrain_GridHeightAt (0x452560) on the same grids, with an origin and the cells' flags 1 and 4.
# usage: terrainheight.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Height,Grid)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x4527f0, 0x452a70), (0x452560, 0x4527f0)])
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


def case(r, ops=None):   # (ops: other benches take the Height ground alone)
    op = r.deal(ops or OPS)
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
    if r.random() < 0.2 and w and h and w < 64 and h < 64:   # a tie on one of the diagonals, with a corner only one of its triangles reads odd
        put(grid, 0x10, '2f', 10.0, 10.0)
        ix, iy = r.randrange(w), r.randrange(h)
        fy = r.choice([0.0, 0.25, 0.5, 0.75])
        fx = r.choice([fy, 1.0 - fy]) if fy else r.choice([0.0, 0.0])
        x, y = ix * 10.0 + fx * 10.0, iy * 10.0 + fy * 10.0
        a = (w + 1) * iy + ix
        put(heights, 4 * r.choice([a, a + 1, a + w + 1, a + w + 2]), 'f', r.choice([float('inf'), float('-inf'), 3e38, -3e38]))
    regions = {0x7a0854: struct.pack('<I', MAP), MAP: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID), GRID: grid, HEIGHTS: heights, CELLS: cells}
    if op == 'Grid':   # 0x452560: an origin (none mostly, whole cells, half a cell, any, NaN) added to the point; cell flags 1 and 4 any mix; any or not
        ox, oy = (r.choice([0.0, 0.0, 0.0, 10.0, -10.0, 5.0, r.uniform(-20, 20), float('nan')]) for _ in range(2))
        put(grid, 8, '2f', ox, oy)
        for i in range(0x800):
            put(cells, 4 * i, 'H', r.getrandbits(16) & ~5 | r.choice([0, 0, 4, 4, 1, 5]))
        x, y = (v + o if o == o and r.random() < 0.9 else v for v, o in ((x, ox), (y, oy)))
        if r.random() < 0.15 and w < 64 and h < 64:   # exactly the far edge in cells (inside the 10-per-cell bound with a negative origin)
            put(grid, 8, '2f', -10.0, -10.0)
            put(grid, 0x10, '2f', 10.0, 10.0)
            x, y = r.choice([(w * 10.0 - 10, y), (x, h * 10.0 - 10), (w * 10.0 - 10, h * 10.0 - 10)])
        return Call(0x452560, [fb(x), fb(y), r.choice([0, 0, 1, 7])], regions, ret='d')
    return Call(0x4527f0, [fb(x), fb(y)], regions, ret='d')


OPS = [k for k in os.environ.get('OPS', 'Height,Grid').split(',') if k]

if __name__ == '__main__':
    B.main(case)
