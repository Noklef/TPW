# Holder_Clone (src/game/model/clone: 0x454a10) against the original: a holder (slot table at
# +0x28) and a model with every array the flags copy, nodes whose pointers lead into them, and
# the parts Model_MarkFaces* then walk; GlobalMem_Alloc serves the block from heap().
# usage: clone.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x454a10, 0x4554d0), (0x44b140, 0x44b220), (0x45b9d0, 0x45ba15)])
HOLDER, MODEL, NODES, OUT = (RAM + 0x1000 * k for k in range(4))
TERRAIN = 0x7a1a8c  # the terrain flags (0x2000: procedural textures could load)
# MODEL page: the model, then its arrays
A50, A54, RECORDS, A58, A5C, A60, A64, A68, PARTS, RUNS = (MODEL + o for o in
                                                           (0x100, 0x200, 0x300, 0x400, 0x600, 0x700, 0x800, 0x900, 0xa00, 0xb00))
PRIMARY, SECONDARY, ROOT = NODES, NODES + 0x800, NODES + 0xc00


def small(r, hi):
    return r.choice([0, 1, 2, r.randrange(hi + 1), r.randrange(hi + 1)])


def into(r, base, n, size):
    """A pointer into an array (or past it, or null, or anywhere)."""
    return r.choice([base + size * r.randrange(max(n, 1)), base + size * r.randrange(max(n, 1)), 0,
                     base + size * r.randrange(max(n, 1)) + r.randrange(size), base - size, r.getrandbits(32)])


def case(r):
    total = r.choice([0, 1, 2, r.randrange(1, 9), r.randrange(1, 9)])
    primary = min(total, r.choice([0, 1, total, r.randrange(total + 1)]))
    n36, n38, n3a, n3c, n3e, n4a = small(r, 6), small(r, 6), small(r, 8), small(r, 12), small(r, 8), small(r, 8)
    nparts = r.choice([0, 0, 1, r.randrange(1, 9)])
    base = r.randrange(0, max(total - nparts, 0) + 1)

    m = bytearray(r.bytes(0x1000))
    mflags = r.getrandbits(32)
    for bit in (8, 0x200, 0x1000, 0x8000, 0x10000):
        mflags = mflags & ~bit | (bit if r.random() < 0.3 else 0)
    put(m, 0x30, 'I', mflags)
    put(m, 0x36, '5H', n36, n38, n3a, n3c, n3e)
    put(m, 0x42, '4H', total, primary, base, nparts)
    put(m, 0x4a, 'H', n4a)
    put(m, 0x50, '7I', A50, A54, A58 if r.random() < 0.9 else 0, A5C, A60, A64, A68)
    for k in range(7):  # sometimes an array is missing
        if r.random() < 0.05:
            put(m, 0x50 + 4 * k, 'I', 0)
    nodes_at = PRIMARY if r.random() < 0.95 else 0
    put(m, 0x70, 'III', nodes_at, SECONDARY if r.random() < 0.95 else 0, r.choice([ROOT, PRIMARY, SECONDARY, ROOT, 0]))
    put(m, 0x7c, 'I', PARTS)
    for k in range(n36):  # +0x54: flag 2 marks a 22-byte record at +0xc
        put(m, A54 - MODEL + 0x10 * k, 'B', r.getrandbits(8))
        put(m, A54 - MODEL + 0x10 * k + 0xc, 'I', r.choice([RECORDS + 0x18 * k, RECORDS + 0x18 * k, 0]))
    keys = [r.getrandbits(32) for _ in range(3)]
    for k in range(nparts):
        put(m, PARTS - MODEL + 0x14 * k + 8, 'I', r.choice(keys + [r.getrandbits(32)]))

    nodes = bytearray(r.bytes(0x1000))
    for k in range(9):  # primary nodes (and the root's spare)
        o = 0xa0 * k if k < 8 else ROOT - NODES
        put(nodes, o, 'I', r.getrandbits(32) & ~(0x80 | 0x4000 | 0x200) | r.choice([0, 0x80]) | r.choice([0, 0, 0x4000]) | r.choice([0, 0x200]))
        put(nodes, o + 4, '3I', *(r.choice([0, PRIMARY + 0xa0 * r.randrange(8), SECONDARY + 0x58 * r.randrange(8), r.getrandbits(32)]) for _ in range(3)))
        put(nodes, o + 0x5a, 'H', small(r, 3))
        put(nodes, o + 0x5e, 'H', small(r, 12))
        put(nodes, o + 0x60, '4I', into(r, A58, n38, 0x30), into(r, A5C, n4a, 12), r.choice([RUNS + 0x80 * k, RUNS, 0]),
            r.choice([A64 + 0x10 * r.randrange(max(n3c - 2, 1)), A64, A64 + 0x10 * r.randrange(max(n3c - 2, 1)) + r.randrange(16)]))
    for k in range(8):  # secondary nodes
        o = SECONDARY - NODES + 0x58 * k
        put(nodes, o, 'I', r.getrandbits(32) & ~0x4000 | r.choice([0, 0, 0x4000]))
        put(nodes, o + 4, '3I', *(r.choice([0, PRIMARY + 0xa0 * r.randrange(8), r.getrandbits(32)]) for _ in range(3)))
    for k in range(16):  # +0x64 entries: +0 into the +0x50 array
        put(m, A64 - MODEL + 0x10 * k, 'I', r.choice([0, into(r, A50, n36, 8)]))

    h = bytearray(r.bytes(0x1000))
    put(h, 4, 'II', MODEL, r.choice([0, HOLDER + 0x80, r.getrandbits(32)]))
    put(h, 0x28, 'I', HOLDER + 0x100)
    blocks = small(r, 3)
    put(h, 0x100, 'IIIII', r.getrandbits(32), HOLDER + 0x200 if r.random() < 0.95 else 0, r.getrandbits(32), blocks,
        HOLDER + 0x400 if r.random() < 0.95 else 0)
    for k in range(8):
        put(h, 0x200 + 0x14 * k, 'I', r.getrandbits(32))

    flags = r.getrandbits(32) & ~0x1fff
    for bit in (1, 4, 8, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x1000):
        if r.random() < 0.35:
            flags |= bit
    mask = r.choice([r.getrandbits(32), 1 << r.randrange(32), 0, 0xffffffff])
    key = r.choice(keys)
    regions = {HOLDER: h, MODEL: m, NODES: nodes, OUT: bytearray(r.bytes(4)), HEAP: heap(0x6000),
               TERRAIN: bytearray(struct.pack('<I', r.getrandbits(32) & ~0x2000 | r.choice([0, 0x2000]))), **global_heap()}
    return Call(0x454a10, [HOLDER, r.getrandbits(32), flags, OUT, key if flags & 0x1000 else mask], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
