# A model's frames fitted to a height grid (src/game/model/gridfit: 0x469a80, 0x469940) against the original. Trees of 1-4 frames
# (0xa0 apart in one array, as the object's frames are) beside their nodes, 1-9 vertices each, put mostly on and near the grid's lines
# (whole cells, a hair off, half way, outside now and then) through identity-like matrices with a translation (any floats now and
# then); grids of 1-6 cells a side with the heights padded for the unchecked reads around a cell; the edge marks at 0x7afe20 (20 x 20)
# with their bounds [0x7afe18] / [0x7b0460] at 1-20; the cursor's index list.
# Ops: Place / Contact (0x469a80, place or not), Model (0x469940).
# usage: gridfit.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, bf, fb  # noqa: E402

B = Bench([(0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)])
GRID, HEIGHTS, FLAGS = RAM, RAM + 0x1000, RAM + 0x2000
FRAMES, NODES, VERTS = RAM + 0x3000, RAM + 0x4000, RAM + 0x5000
CURSOR, OBJECT = RAM + 0x7000, RAM + 0x7800
MARKS = 0x7afe18   # [0x7afe18] columns, 0x7afe20 the marks (20 dwords a row), [0x7b0460] rows
PAD = 0x100
FRAME = 0xa0


def matrix(r, t):
    # (finite and affine: Matrix_Invert's pivot search reads outside its rows on a NaN, at a stack address that only the emulator faults on)
    m = [fb(1.0 if i in (0, 5, 10, 15) else 0.0) for i in range(16)]
    if r.random() < 0.08:
        m[0:3], m[4:7], m[8:11] = ([fb(r.uniform(-2, 2)) for _ in range(3)] for _ in range(3))
    if r.random() < 0.15:
        for i in r.sample(range(12), 2):
            m[i] = fb(r.uniform(-1, 1))
    m[12:15] = [fb(x) for x in t]
    return m


def coordinate(r, origin, cell, n):
    u = r.choice([float(r.randrange(n + 1)), r.randrange(n + 1) + r.choice([-1, 1]) * r.choice([0.003, 0.05, 0.2]),
                  r.randrange(n) + 0.5, r.uniform(0, n), r.uniform(-0.6, n + 0.6), n - 1e-3])
    return origin + u * cell


def case(r, op=None):
    op = op or r.deal(['Place', 'Place', 'Contact', 'Contact', 'Model'])
    w, h = r.randrange(1, 7), r.randrange(1, 7)
    cell = [r.uniform(0.5, 8), r.uniform(0.5, 8)]
    origin = [r.uniform(-50, 50), r.uniform(-50, 50)]
    grid = bytearray(r.bytes(0x30))
    struct.pack_into('<4f2I', grid, 8, origin[0], origin[1], cell[0], cell[1], w, h)
    struct.pack_into('<2I', grid, 0x28, HEIGHTS + PAD, FLAGS)
    if r.random() < 0.03:
        struct.pack_into('<I', grid, r.choice([8, 0xc, 0x10, 0x14]), r.f())
    nheights = (w + 1) * (h + 2) + 1
    heights = bytearray(PAD + 4 * nheights + PAD)
    scale = r.choice([1, 20, None])
    for k in range(0, len(heights), 4):
        struct.pack_into('<I', heights, k, r.f(scale) if r.random() < 0.97 else r.f(scale, special=1))
    flags = bytearray(r.getrandbits(8) & 4 for _ in range(4 * (w + 1) * (h + 2)))
    marks = bytearray(r.bytes(0x64c))
    struct.pack_into('<I', marks, 0, r.randrange(1, 21))
    struct.pack_into('<I', marks, 0x648, r.randrange(1, 21))
    shift = [r.uniform(-20, 20) for _ in range(3)] if r.random() < 0.7 else [0.0, 0.0, 0.0]
    # the tree: frame k at FRAMES + k * 0xa0, node k at NODES + k * 0x70; children of 0 (and of 1)
    n = r.choice([1, 1, 2, 3, 4])
    parent = [None] + [r.choice([0, k - 1]) for k in range(1, n)]
    frames, nodes, verts = bytearray(FRAME * n), bytearray(0x70 * n), bytearray()
    contact = op == 'Contact'
    for k in range(n):
        fo, no = FRAME * k, 0x70 * k
        frames[fo:fo + FRAME] = r.bytes(FRAME)
        nodes[no:no + 0x70] = r.bytes(0x70)
        kids = [j for j in range(n) if parent[j] == k]
        for buf, off, size, base in ((frames, fo, FRAME, FRAMES), (nodes, no, 0x70, NODES)):
            struct.pack_into('<I', buf, off + 0xc, base + size * kids[0] if kids else 0)
            sibs = [j for j in range(n) if parent[j] == parent[k] and j > k] if k else []
            struct.pack_into('<I', buf, off + 8, base + size * sibs[0] if sibs else 0)
        bits = r.getrandbits(32) & ~0x21200
        if r.random() < 0.25:
            bits |= 0x20000
        if r.random() < 0.6:
            bits |= 0x1000
        if r.random() < 0.06:
            bits |= 0x200
        struct.pack_into('<I', frames, fo, bits)
        struct.pack_into('<16I', frames, fo + 0x10, *matrix(r, [0.0, 0.0, 0.0] if r.random() < 0.7 else [r.uniform(-3, 3) for _ in range(3)]))
        count = r.randrange(0, 10) if r.random() < 0.1 else r.randrange(1, 10)
        struct.pack_into('<H', frames, fo + 0x58, count)
        groups = (count + 3) // 4 or 1
        nv, fv = VERTS + len(verts), VERTS + len(verts) + 0x30 * groups
        node_v = bytearray(0x30 * groups)
        for i in range(4 * groups):
            g, lane = i // 4 * 0x30, i % 4
            x = coordinate(r, origin[0], cell[0], w) - shift[0]
            z = coordinate(r, origin[1], cell[1], h) - shift[2]
            y = (r.choice([0.0, r.uniform(-0.02, 0.02), r.uniform(-200, 200)]) if contact else r.uniform(-50, 50)) - shift[1]
            for axis, v in enumerate((x, y, z)):
                struct.pack_into('<I', node_v, g + lane * 4 + axis * 0x10, fb(v) if r.random() < 0.98 else r.f())
        verts += node_v + r.bytes(0x30 * groups)
        struct.pack_into('<I', nodes, no + 0x60, nv)
        struct.pack_into('<I', frames, fo + 0x60, fv)
        for j in range(3):   # the box: low corner, high corner
            lo, hi = r.uniform(-60, 0), r.uniform(0, 60)
            struct.pack_into('<f', frames, fo + 0x78 + 4 * j, lo)
            struct.pack_into('<f', frames, fo + 0x84 + 4 * j, hi)
    if r.random() < 0.03 and n > 1:   # the frames' chain ends before the nodes' (crashes in both)
        struct.pack_into('<I', frames, 0xc, 0)
    total = sum(struct.unpack_from('<H', frames, FRAME * k + 0x58)[0] for k in range(n))
    index = [r.randrange(10) for _ in range(total + 8)]
    cursor = bytearray(struct.pack('<I', CURSOR + 0x10)) + bytearray(12) + struct.pack('<%dH' % len(index), *index)
    cursor += bytearray(0x10)
    hits_at, best_at = CURSOR + len(cursor), CURSOR + len(cursor) + 4
    cursor += struct.pack('<iIi', r.randrange(-3, 50), r.getrandbits(32) | 0x80000000, r.randrange(-1, 3))
    regions = {GRID: grid, HEIGHTS: heights, FLAGS: flags, FRAMES: frames, NODES: nodes, VERTS: verts, CURSOR: cursor, MARKS: marks}
    lift = r.f(5) if r.random() < 0.9 else r.f()
    if op == 'Model':
        # object +4 the root, +8 -> +0x14 the cursor; the root: +0x30 flags, +0x44 frames, +0x70 the frames, +0x78 the root frame,
        # +0x9c the holder (+0x78 the root node); each frame's +0x98 a sub-object (+4 its cursor) or none
        obj = bytearray(r.bytes(0x200))
        root, holder, sub, other = OBJECT + 0x20, OBJECT + 0x100, OBJECT + 0x1c0, OBJECT + 0x1e0
        struct.pack_into('<2I', obj, 4, root, other)
        struct.pack_into('<I', obj, 0x1e0 + 0x14, CURSOR + 0x10)
        struct.pack_into('<H', obj, 0x20 + 0x44, r.randrange(0, n + 1))
        struct.pack_into('<I', obj, 0x20 + 0x70, FRAMES)
        struct.pack_into('<I', obj, 0x20 + 0x78, FRAMES)
        struct.pack_into('<I', obj, 0x20 + 0x9c, holder)
        struct.pack_into('<I', obj, 0x100 + 0x78, NODES)
        struct.pack_into('<I', obj, 0x1c0 + 4, CURSOR + 0x10 + 2 * r.randrange(4))
        for k in range(n):
            struct.pack_into('<I', frames, FRAME * k + 0x98, sub if r.random() < 0.7 else 0)
        owner = bytearray(r.bytes(0x70))
        struct.pack_into('<I', owner, 0x6c, GRID if r.random() < 0.95 else 0)
        regions[OBJECT] = obj
        regions[OBJECT + 0x200] = owner
        return Call(0x469940, [OBJECT, OBJECT + 0x200 if r.random() < 0.97 else 0, lift, r.getrandbits(3) & r.choice([0, 1, 5, 7])], regions)
    m = matrix(r, shift)
    only = 0
    if r.random() < 0.2:
        only = FRAMES + FRAME * r.randrange(n) if r.random() < 0.8 else CURSOR
    flags_arg = r.getrandbits(3) if r.random() < 0.3 else r.getrandbits(1)
    best = r.random() < 0.6
    args = [NODES, FRAMES, CURSOR] + m + [GRID, int(op == 'Place') * r.choice([1, 1, 0x80000000]), hits_at, lift,
                                          best_at if best else 0, hits_at + 8 if best else 0, flags_arg, only, r.choice([0, 1])]
    return Call(0x469a80, args, regions)


if __name__ == '__main__':
    B.main(case)
