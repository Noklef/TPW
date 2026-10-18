# The terrain model's normals (src/game/terrain/normals: Terrain_ScaleNormals 0x450b30, Terrain_ShadeNormals 0x450d10) against the
# originals, over Matrix_PreMul, WorldVerts_TransformNode, Matrix_Identity, MapCell_HasFlags and SceneFlags_Touch. A tree of one to five
# nodes (flag 0x200 now and then; matrices the identity, turned by right angles or scaled, moved about the map, or random), each with up
# to 8 vertices about the map and up to 8 normals (about unit length, zero, tiny, huge or NaN; whole vectors zero, too long for a float
# once squared, denormal, or NaNs of either sign and like significands) whose vertex indices reach the block's
# first 0x40 entries (the node's own positions or older ones, about the map, now and then NaN or negative); a grid of 4 - 24 by 2 - 8
# cells whose flags (+0x2c) have 0x400 or not, the table at 0x7a0884 that one or another, the length at 0x74cf30 0.225 or not. Scale: the
# root with a matrix (the identity, moved, or random); Shade: the map's tree, the 0x44-byte map cells' flags 0x40 / 0x80 any mix, the
# map's size (0x7a0314 / 0x7a0374) the grid's or less.
# Cells / ModelCells (src/game/terrain/nodecells: Terrain_NodeCells 0x46eeb0, Terrain_ModelCells 0x46f0c0, over Matrix_TransformPoint): the
# same trees with boxes about the map (either way round on each axis, now and then NaN, on a cell's edge or below 0), children skipped by
# flag 0x40000000 now and then; the grid's origin 0 or moved (or NaN), its cells 10 (or 5, 0.3, 0, NaN), its size 4 - 24 by 2 - 8.
# usage: normals.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x450b30, 0x450d0d), (0x450d10, 0x450e1c), (0x4702d0, 0x4704c0), (0x466c80, 0x466d62), (0x46f150, 0x46f176),
           (0x535db0, 0x535dc3), (0x5836b0, 0x5836c2), (0x46eeb0, 0x46f0bf), (0x46f0c0, 0x46f0f9), (0x46fc00, 0x46fca0)])
NODES, BLOCK, MAP, GRID, FLAGS, TABLE, PARTS = RAM, RAM + 0x800, RAM + 0xc00, RAM + 0xd00, RAM + 0x1000, RAM + 0x1400, RAM + 0x2000
WORLD = 0x32000000
NBLOCK = 0x40


def odd(r, usual):
    p = r.random()
    if p < 0.03:
        return float('nan')
    if p < 0.06:
        return r.choice([0.0, -0.0, 1e-30, 1e30, -1e30, 3e38])
    return usual


def matrix(r, spread):
    """The identity turned by a right angle about y (or scaled), moved by up to spread along x and z; or random."""
    if r.random() < 0.1:
        return [odd(r, r.uniform(-2, 2)) for _ in range(16)]
    m = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0]
    turn = r.choice([0, 0, 1, 2, 3])
    c, s = [(1, 0), (0, 1), (-1, 0), (0, -1)][turn]
    m[0], m[2], m[8], m[10] = float(c), float(-s), float(s), float(c)
    if r.random() < 0.2:
        k = r.choice([0.5, 2.0, r.uniform(0.1, 3)])
        m = [v * k if i % 4 != 3 else v for i, v in enumerate(m)]
    m[12], m[13], m[14] = r.uniform(0, spread), r.uniform(-5, 5), r.uniform(0, spread)
    return m


def tree(r, regions):
    count = r.choice([1, 1, 2, 3, 5])
    nodes = bytearray(0x100 * count)
    first, last = {}, {}
    for k in range(count):
        base = 0x100 * k
        nodes[base:base + 0x100] = r.bytes(0x100)
        put(nodes, base, 'I', r.getrandbits(32) & ~0x200 | (0x200 if r.random() < 0.15 else 0))
        put(nodes, base + 8, 'I', 0)
        put(nodes, base + 0xc, 'I', 0)
        put(nodes, base + 0x10, '16f', *matrix(r, 60.0 if k == 0 else 30.0))
        if k:   # a child of an earlier node, after its other children
            parent = r.randrange(k)
            if parent in last:
                put(nodes, 0x100 * last[parent] + 8, 'I', NODES + base)
            else:
                put(nodes, 0x100 * parent + 0xc, 'I', NODES + base)
            last[parent] = k
        nv, nn = r.choice([0, 1, 4, 5, 8]), r.choice([0, 1, 3, 8, 8])
        parts = PARTS + 0x400 * k
        verts = bytearray(0x30 * 2 + 0x10)
        for i in range(len(verts) // 4):
            put(verts, 4 * i, 'f', odd(r, r.uniform(0, 100)))
        normals = bytearray(12 * 8)
        for i in range(24):
            put(normals, 4 * i, 'f', odd(r, r.uniform(-1, 1)))
        for i in range(8):   # whole vectors: zero, past float range when squared and summed, denormal; NaNs of either sign
            if r.random() < 0.08:
                put(normals, 12 * i, '3f', *r.choice([(0.0, 0.0, 0.0), (3e38, -3e38, 3e38), (2e38, 2e38, 1e38), (1e-40, -1e-40, 1e-40),
                                                       (3e-39, 0.0, 1e-41)]))
            if r.random() < 0.05:
                put(normals, 12 * i, '3I', *(r.choice([0x7fc00000, 0xffc00000, 0x7fc00001, 0x3f000000]) for _ in range(3)))
        indices = bytearray(2 * 8)
        for i in range(8):
            put(indices, 2 * i, 'H', r.choice([r.randrange(max(nv, 1)), r.randrange(max(nv, 1)), r.randrange(NBLOCK)]))
        put(nodes, base + 0x58, 'H', nv)
        put(nodes, base + 0x5e, 'H', nn)
        put(nodes, base + 0x60, 'I', parts)
        put(nodes, base + 0x64, 'I', parts + 0x100)
        put(nodes, base + 0x94, 'I', parts + 0x200)
        regions.update({parts: verts, parts + 0x100: normals, parts + 0x200: indices})
    regions[NODES] = nodes


def setup(r):
    regions = {}
    tree(r, regions)
    block = bytearray(12 * NBLOCK)
    for i in range(3 * NBLOCK):
        put(block, 4 * i, 'f', odd(r, r.uniform(-10, 250)))
    width, height = r.choice([4, 8, 16, 24]), r.choice([2, 4, 8])
    flags = bytearray(4 * width * height)
    for i in range(width * height):
        put(flags, 4 * i, 'I', r.getrandbits(32) & ~0x400 | r.choice([0, 0x400]))
    table = bytearray(flags) if r.random() < 0.5 else bytearray(r.bytes(len(flags)))
    regions.update({0x7aecd4: struct.pack('<I', BLOCK), BLOCK: block, 0x7a0854: struct.pack('<I', MAP),
                    MAP: put(put(bytearray(r.bytes(0x80)), 0x6c, 'I', GRID), 0x78, 'I', NODES),
                    GRID: put(put(bytearray(r.bytes(0x30)), 0x18, '2I', width, height), 0x2c, 'I', FLAGS), FLAGS: flags,
                    0x7a0884: struct.pack('<I', r.choice([FLAGS, TABLE])), TABLE: table,
                    0x74cf30: struct.pack('<f', r.choice([0.225, 0.225, 0.225, 2.0, odd(r, r.uniform(0, 3))]))})
    return regions, width, height


def scale(r):
    regions, _, _ = setup(r)
    m = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0] if r.random() < 0.4 else matrix(r, 40.0)
    return Call(0x450b30, [NODES] + list(struct.unpack('<16I', struct.pack('<16f', *m))), regions)


def shade(r):
    regions, width, height = setup(r)
    cells = bytearray(r.bytes(0x294 + 0x44 * ((height << 7) + 0x20)))
    for k in range(len(cells) // 0x44 - 10):
        put(cells, 0x294 + 0x44 * k + 0xe, 'H', r.getrandbits(16) & ~0xc0 | r.choice([0, 0x40, 0x40, 0x80, 0xc0]))
    regions.update({WORLD: cells, RAM + 0x7f00: struct.pack('<I', WORLD), 0x7cf6ec: struct.pack('<I', RAM + 0x7f00),
                    0x7a0314: struct.pack('<i', r.choice([width, width, 2, 0x80])), 0x7a0374: struct.pack('<i', r.choice([height, height, 1, 0x80])),
                    0x8bd508: r.bytes(8), 0x8bcc40: r.bytes(4), 0x79f7cc: r.bytes(4)})
    return Call(0x450d10, [], regions)


def boxes(r, regions):
    nodes = regions[NODES]
    for base in range(0, len(nodes), 0x100):
        lo = [odd(r, r.choice([r.uniform(-20, 200), float(r.randrange(-2, 20) * 10), -0.0])) for _ in range(3)]
        hi = [odd(r, v + r.choice([r.uniform(-5, 60), 10.0, 0.0])) if v == v else 1.0 for v in lo]
        put(nodes, base + 0x78, '6f', *(lo + hi))
        if base and r.random() < 0.15:
            put(nodes, base, 'I', struct.unpack_from('<I', nodes, base)[0] | 0x40000000)
    grid = regions[GRID]
    put(grid, 8, '2f', *(r.choice([0.0, 0.0, 0.0, r.uniform(-30, 30), float('nan')]) for _ in range(2)))
    put(grid, 0x10, '2f', *(r.choice([10.0, 10.0, 10.0, 5.0, 0.3, 0.0, float('nan')]) for _ in range(2)))


def cells(r, op):
    regions, _, _ = setup(r)
    boxes(r, regions)
    if op == 'ModelCells':
        return Call(0x46f0c0, [MAP], regions)
    m = [1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0, 0, 0, 0, 0, 1.0] if r.random() < 0.4 else matrix(r, 40.0)
    return Call(0x46eeb0, [NODES, GRID] + list(struct.unpack('<16I', struct.pack('<16f', *m))), regions)


CASES = {'Scale': scale, 'Shade': shade, 'Cells': lambda r: cells(r, 'Cells'), 'ModelCells': lambda r: cells(r, 'ModelCells')}


def case(r):
    return CASES[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
