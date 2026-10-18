# A model's height map (src/game/model/heightmap: HeightMap_Rasterize 0x451e20) against the original: a node and up to two
# children (+0xc, +8), each with a matrix (a turn about y, a move), vertices about the grid (some past its edges, below 0 or
# above 100), groups of faces through the remap, and flags with 0x8210 now and then; the grid's box, cells and size (width
# 0x79fb20, depth 0x79fb08, row stride 0x79f7d4) and the row table at 0x79f7e8 between them; *hit given or not, *clipped.
# usage: heightmap.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x451e20, 0x452533), (0x4702d0, 0x4704c0), (0x46fc00, 0x46fca0)])
GRID, CELLS, HIT, CLIP, NODES = RAM, RAM + 0x100, RAM + 0x4000, RAM + 0x4010, RAM + 0x8000
CALLS = {'Raster': 0x451e20}


def matrix(r, turn, move):
    a = r.choice([0.0, 0.0, r.uniform(-math.pi, math.pi)]) if turn else 0.0
    c, s = math.cos(a), math.sin(a)
    m = [c, 0.0, -s, 0.0, 0.0, 1.0, 0.0, 0.0, s, 0.0, c, 0.0, move[0], move[1], move[2], 1.0]
    if r.random() < 0.1:   # (a scale)
        k = r.choice([0.5, 2.0, -1.0])
        m = [v * k if i < 11 else v for i, v in enumerate(m)]
    return [fb(v) for v in m]


def node(r, base, at, width, depth, child, sibling, top):
    """A node at base (0x200 bytes), its vertices, groups, faces and remap at base + 0x200.. (all in buf at offset at)."""
    buf = bytearray(r.bytes(0x800))
    count = r.choice([1, 3, 4, 5, 8, 12])
    flags = r.getrandbits(32) & ~0x8210 | (r.choice([0x10, 0x200, 0x8000]) if r.random() < 0.1 else 0)
    move = (width / 2 if top else r.uniform(-3, 3), r.choice([0.0, r.uniform(0, 20)]) if top else r.uniform(-5, 5), depth / 2 if top else r.uniform(-3, 3))
    put(buf, 0, 'I', flags)
    put(buf, 8, 'II', sibling, child)
    put(buf, 0x10, '16I', *matrix(r, True, move))
    verts, groups, faces, remap = base + 0x200, base + 0x400, base + 0x500, base + 0x700
    for i in range(count):
        b, lane = 0x200 + (i >> 2) * 0x30, i & 3
        spread = r.choice([0.25, 0.35, 0.4, 0.45, 0.55])
        x, y, z = r.uniform(-width * spread, width * spread), r.uniform(-30, 110), r.uniform(-depth * spread, depth * spread)
        if r.random() < 0.05:
            x = r.choice([0.0, -width / 2 - 0.5, width / 2 + 0.5, 1e6, -width / 2 - 1.0])
        if r.random() < 0.05:   # (on the edges the checks compare against exactly: -1 past the grid, a height of 100)
            y, z = r.choice([(100.0, z), (99.0, z), (y, -depth / 2 - 1.0)])
        put(buf, b + 4 * lane, 'f', x)
        put(buf, b + 0x10 + 4 * lane, 'f', y)
        put(buf, b + 0x20 + 4 * lane, 'f', z)
    nremap = r.choice([3, 6, 9])
    for i in range(nremap):
        put(buf, 0x700 + 2 * i, 'H', r.randrange(count))
    ngroups = r.choice([0, 1, 1, 2, 3])
    total = 0
    for k in range(ngroups):
        n = r.choice([0, 1, 1, 2, 4])
        put(buf, 0x400 + 0x10 * k + 4, 'HH', total, n)
        total += n
    for f in range(total):
        put(buf, 0x500 + 8 * f, 'HHHH', r.getrandbits(16), *[r.randrange(nremap) for _ in range(3)])
    put(buf, 0x58, 'HH', count, ngroups)
    put(buf, 0x60, 'I', verts)
    put(buf, 0x6c, 'II', groups, faces)
    put(buf, 0x94, 'I', remap)
    return buf


def case(r):
    op = r.deal(OPS)
    width, depth = r.choice([16.0, 20.0, 40.0, 63.5, 97.5]), r.choice([16.0, 20.0, 40.0, 64.0, 98.0])
    stride = int(width / 2) + 2 + r.choice([0, 0, 3])
    kids = r.choice([0, 0, 1, 2])
    bases = [NODES + 0x800 * i for i in range(kids + 1)]
    regions = {}
    for i, base in enumerate(bases):
        child = bases[1] if i == 0 and kids else 0
        sibling = bases[i + 1] if i and i + 1 < len(bases) else 0
        regions[base] = node(r, base, i, width, depth, child, sibling, i == 0)
    glob = bytearray(r.bytes(0x350))   # 0x79f7d4: the stride, the row table (+0x14), the depth (+0x334), the width (+0x34c)
    put(glob, 0, 'i', stride)
    put(glob, 0x334, 'f', depth)
    put(glob, 0x34c, 'f', width)
    grid = bytearray(r.bytes(0x40))
    put(grid, 0xc, 'I', CELLS)
    lo, hi = r.choice([(1e9, -1e9), (0.0, 0.0), (5.0, 30.0)])
    put(grid, 0x18, '6f', lo, lo, lo, hi, hi, hi)
    rows = int(depth / 2) + 2
    regions.update({0x79f7d4: glob, GRID: grid, CELLS: r.bytes(stride * rows), HIT: r.bytes(4),
                    CLIP: struct.pack('<I', r.choice([0, 0, 1]))})
    top = matrix(r, r.random() < 0.3, (r.uniform(-2, 2), 0.0, r.uniform(-2, 2)) if r.random() < 0.3 else (0.0, 0.0, 0.0))
    args = [bases[0], r.getrandbits(32)] + top + [GRID, r.choice([HIT, HIT, 0]), r.getrandbits(32), r.getrandbits(32), CLIP]
    return Call(CALLS[op], args, regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
