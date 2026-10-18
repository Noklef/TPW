# The string maps record's destructor of src/game/core/stringmapsrecord (0x4e54c0) against the original: two wide strings (null, counted,
# frozen), three valid red-black trees (stdtree.py's) of 0-5 nodes holding counted strings (kinds D, C and E, each with its own nil node and
# map counter: 1 so the nil node is freed, more, or 0) and a list of 0-3 counted-buffer nodes.
# usage: stringmapsrecord.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import stdtree  # noqa: E402
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

B = Bench([(0x4e54c0, 0x4e5700), (0x4e5800, 0x4e596b), (0x4f2590, 0x4f26f8), (0x4f2960, 0x4f2ac8), (0x4f3290, 0x4f33f8), (0x4f4070, 0x4f4120),
           (0x4f4ac0, 0x4f4b70), (0x4f4fd0, 0x4f5080), (0x4f3b10, 0x4f4070), (0x4f4580, 0x4f4ac0), (0x4f2d50, 0x4f3290), (0x4f44d0, 0x4f4580),
           (0x4f4f20, 0x4f4fd0), (0x4f5430, 0x4f54e0), (0x4f54e0, 0x4f5560), (0x4f23f0, 0x4f2450), (0x5ce530, 0x5ce53e), (0x5ce910, 0x5ce91f),
           (0x67acb0, 0x67acb1)],
          hooks={0x691de0: lambda e: e.r.__setitem__('EAX', e.r['ECX']), 0x691ea0: lambda e: None})
REC, LNODES, POOL, GLOB = RAM, RAM + 0x3000, RAM + 0x5000, 0x7ced20
HEADS, NILS, NODES = RAM + 0x200, RAM + 0x400, RAM + 0x10000
# kind: (offset in the record, nil global, map counter, colour offset)
TREES = [('D', 0x20, 0x7ced24, 0x7ced28, 0x1c), ('C', 0x30, 0x7ced2c, 0x7ced30, 0x20), ('E', 0x40, 0x7ced34, 0x7ced38, 0x1c)]
SLOTS = 32


def block(r, pool, slots):
    """A counted wide string's characters in the next pool slot (its count byte before them: shared, one owner or frozen), or null."""
    if r.random() < 0.15:
        return 0
    k = slots.pop()
    pool[0x40 * k + 1] = r.choice([0, 0, 1, 2, 0xff])
    return POOL + 0x40 * k + 2


def case(r):
    pool = bytearray(r.bytes(0x40 * SLOTS))
    slots = list(range(SLOTS))
    r.shuffle(slots)
    rec = bytearray(r.bytes(0x60))
    glob = bytearray(r.bytes(0x20))
    regions = {HEAP: heap()}
    for t, (_kind, at, nil_at, counter, colour) in enumerate(TREES):
        head_at, nil, base = HEADS + 0x40 * t, NILS + 0x40 * t, NODES + 0x1000 * t
        n = r.choice([0, 0, 1, 2, 3, 5])
        nodes, root = stdtree.build(r, n) if n else ({}, None)
        keys = sorted(nodes)
        addr = {k: base + 0x40 * i for i, k in enumerate(keys)}
        ptr = lambda k: nil if k is None else addr[k]  # noqa: E731
        blob = bytearray(r.bytes(0x40 * max(n, 1)))
        for k in keys:
            o = addr[k] - base
            struct.pack_into('<III', blob, o, ptr(nodes[k][0]), head_at if nodes[k][1] is None else addr[nodes[k][1]], ptr(nodes[k][2]))
            p = block(r, pool, slots)
            struct.pack_into('<III', blob, o + 0x10, p, r.choice([0, 1, 5]), r.choice([0, 31, 40]))
            struct.pack_into('<I', blob, o + colour, nodes[k][3])
        regions[base] = blob
        first, top, last = (addr[keys[0]], addr[root], addr[keys[-1]]) if keys else (head_at, nil, head_at)
        regions[head_at] = struct.pack('<III', first, top, last) + r.bytes(0x20)
        nilb = bytearray(r.bytes(0x40))
        struct.pack_into('<I', nilb, colour, stdtree.BLACK)
        regions[nil] = nilb
        struct.pack_into('<III', rec, at + 4, head_at, r.getrandbits(32), n)
        struct.pack_into('<I', glob, nil_at - GLOB, nil)
        struct.pack_into('<I', glob, counter - GLOB, r.choice([1, 1, 2, 5, 0]))
    k = r.choice([0, 1, 2, 3])
    at = [LNODES + 0x18 * i for i in range(k + 1)]
    lnodes = bytearray(r.bytes(0x18 * 4))
    for i in range(k + 1):
        struct.pack_into('<II', lnodes, at[i] - LNODES, at[(i + 1) % (k + 1)], at[i - 1])
    for i in range(1, k + 1):
        struct.pack_into('<I', lnodes, at[i] - LNODES + 0xc, block(r, pool, slots))
    struct.pack_into('<II', rec, 0x58, at[0], r.choice([k, k, k + 1, 0]))
    for o in (0, 0x10):
        struct.pack_into('<III', rec, o + 4, block(r, pool, slots), r.choice([0, 1, 5]), r.choice([0, 31, 40]))
    regions.update({REC: rec, GLOB: glob, LNODES: lnodes, POOL: pool})
    return Call(0x4e54c0, [REC], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
