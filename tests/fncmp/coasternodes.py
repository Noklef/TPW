# The coaster's nodes sampled from the track (src/game/coaster/nodes: Coaster_BuildNodes 0x440c10) against the original: a
# chain of 1-4 pieces (+0x64 on, +0x68 back, +0x90 -> {count, spline nodes, second nodes}, +0xfc, +8 flags, +0x74 a link or
# none) whose nodes follow a path (positions, controls either side, +0x14 back / +0x18 on, +0x10 the piece, +8 a value), the
# ring of cars at coaster+0x54 (the node after the last piece; Coaster_SetSegmentWindow), and the coaster's capacity, block and
# spacing: a block already big enough, or one got from the GlobalAlloc heap (none, small or the old one freed).
# usage: coasternodes.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, global_heap, heap, put  # noqa: E402

B = Bench([(0x440c10, 0x4417f1), (0x440ad0, 0x440c10), (0x43dbf0, 0x43ea90), (0x46f210, 0x46f240), (0x470940, 0x470990),
           (0x43b750, 0x43b8b0), (0x45b9d0, 0x45ba30), (0x67b410, 0x67b440)], sines=256)
TRACK, PIECES, LISTS, NODES, NODES2, CARS, CARLISTS, CARNODES, COASTER, TYPE, LINK, BUF = (
    RAM, RAM + 0x1000, RAM + 0x1800, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000, RAM + 0x4400, RAM + 0x4800, RAM + 0x5000,
    RAM + 0x5400, RAM + 0x5800, RAM + 0x10000)
SLOTS = 0x400   # (BUF's nodes)
CALLS = {'Build': 0x440c10}


def path_nodes(r, buf, count, start, base, lift):
    """count nodes at buf + base + 0x58 i along a path from start: position +0x40, controls +0x34 (behind) / +0x4c (ahead)."""
    x, y, z = start
    for i in range(count):
        dx, dy, dz = r.uniform(-4, 4), r.uniform(-1, 1), r.uniform(0.5, 5)
        if r.random() < 0.1:   # (a node on the last one: no way between them)
            dx = dy = dz = 0.0
        x, y, z = x + dx, y + dy, z + dz
        b = base + 0x58 * i
        k = r.choice([0.3, 0.3, 0.5, 0.0, r.uniform(0, 1.5)])
        put(buf, b + 0x40, '3f', x, y + lift, z)
        put(buf, b + 0x34, '3f', x - dx * k + r.uniform(-0.2, 0.2), y + lift - dy * k, z - dz * k)
        put(buf, b + 0x4c, '3f', x + dx * k, y + lift + dy * k + r.uniform(-0.2, 0.2), z + dz * k)
        put(buf, b + 8, 'f', r.choice([0.0, 0.0, 1.0, 2.0, 2.5, float(r.randrange(8)), r.uniform(-1, 9)]))
    return x, y, z


def case(r):
    op = r.deal(OPS)
    n = r.choice([1, 2, 2, 3, 3, 4])
    counts = [r.choice([1, 1, 2, 2, 3, 4]) for _ in range(n)]
    if n == 1 or counts[0] == 1 and r.random() < 0.9:   # (one node and no next piece faults)
        counts[0] = r.choice([2, 3, 4])
    pieces, lists = bytearray(r.bytes(0x100 * n)), bytearray(r.bytes(0x10 * n))
    nodes, nodes2 = bytearray(r.bytes(0x400 * n)), bytearray(r.bytes(0x400 * n))
    order = [(k, i) for k in range(n) for i in range(counts[k])]
    pos = (r.uniform(-50, 50), r.uniform(0, 20), r.uniform(-50, 50))
    for k in range(n):
        p = PIECES + 0x100 * k
        put(pieces, 0x100 * k + 8, 'I', r.getrandbits(32) & ~0x2000 | r.choice([0, 0, 0x2000]))
        put(pieces, 0x100 * k + 0x64, 'II', PIECES + 0x100 * (k + 1) if k + 1 < n else 0, PIECES + 0x100 * (k - 1) if k else 0)
        put(pieces, 0x100 * k + 0x74, 'I', r.choice([0, LINK, LINK]))
        put(pieces, 0x100 * k + 0x90, 'I', LISTS + 0x10 * k)
        put(pieces, 0x100 * k + 0xfc, 'i', r.choice([0, k, 2 * k, r.randrange(-3, 12)]))
        put(lists, 0x10 * k, 'III', counts[k], NODES + 0x400 * k, NODES2 + 0x400 * k)
        pos = path_nodes(r, nodes, counts[k], pos, 0x400 * k, 0.0)
        r2 = r.uniform(0.5, 2)
        for i in range(counts[k]):   # (the second nodes: the same path, raised, or one of their own)
            b = 0x400 * k + 0x58 * i
            nodes2[b:b + 0x58] = nodes[b:b + 0x58]
            for off in (0x38, 0x44, 0x50):
                put(nodes2, b + off, 'f', struct.unpack_from('<f', nodes, b + off)[0] + r2)
            if r.random() < 0.1:
                path_nodes(r, nodes2, 1, (0.0, 0.0, 0.0), b, 1.0)
    for j, (k, i) in enumerate(order):   # +0x10 the piece, +0x14 the node before (or none), +0x18 the one after
        b = 0x400 * k + 0x58 * i
        for buf, at in ((nodes, NODES), (nodes2, NODES2)):
            prev = order[j - 1] if j else None
            nxt = order[j + 1] if j + 1 < len(order) else (k, i)
            put(buf, b + 0x10, 'I', PIECES + 0x100 * k)
            put(buf, b + 0x14, 'I', at + 0x400 * prev[0] + 0x58 * prev[1] if prev and r.random() < 0.9 else 0)
            put(buf, b + 0x18, 'I', at + 0x400 * nxt[0] + 0x58 * nxt[1] if r.random() < 0.95 else CARNODES)
    cars, carlists, carnodes = bytearray(r.bytes(0x300)), bytearray(r.bytes(0x30)), bytearray(r.bytes(3 * 0x58))
    for k in range(3):   # the cars' ring: +0x64 on, +0x90 -> {1, node k, node k}
        put(cars, 0x100 * k + 0x64, 'I', CARS + 0x100 * ((k + 1) % 3))
        put(cars, 0x100 * k + 0x90, 'I', CARLISTS + 0x10 * k)
        put(carlists, 0x10 * k, 'III', 1, CARNODES + 0x58 * k, CARNODES + 0x58 * k)
    path_nodes(r, carnodes, 3, pos, 0, 0.0)
    for k in range(3):
        put(carnodes, 0x58 * k + 0x18, 'I', CARNODES + 0x58 * ((k + 1) % 3))
    coaster = bytearray(r.bytes(0x200))
    put(coaster, 4, 'I', TYPE)
    put(coaster, 0x54, 'I', CARS)
    spacing = r.choice([0.5, 1.0, 1.0, 2.0, 3.3, 7.5, 25.0, r.uniform(0.4, 10)])
    if r.random() < 0.04:
        spacing = r.choice([0.0, 1e-3])   # (not NaN, inf or below 0: the walk never ends, in the original too)
    put(coaster, 0xac, 'f', spacing)
    put(coaster, 0xa8, 'f', r.choice([0.0, 3.0, 2.5, r.uniform(-2, 40)]))   # (kept when the last piece leaves it)
    regions = {TRACK: put(bytearray(r.bytes(0x20)), 0x18, 'I', PIECES), 0x790fe4: struct.pack('<I', TRACK), PIECES: pieces,
               LISTS: lists, NODES: nodes, NODES2: nodes2, CARS: cars, CARLISTS: carlists, CARNODES: carnodes, COASTER: coaster,
               TYPE: put(bytearray(r.bytes(0x400)), 0x31c, 'f', r.choice([0.0, 0.5, 1.0, r.uniform(-1, 3)])),
               LINK: struct.pack('<I', r.choice([0, 1, 2, 3])), HEAP: heap(0x40000, r.choice([0] * 9 + [1])), **global_heap()}
    if r.random() < 0.6:   # a block big enough already
        put(coaster, 0x98, 'II', SLOTS, BUF)
        regions[BUF] = bytearray(r.bytes(0x38 * (SLOTS + 1)))
    else:   # none, a small one (freed), or one too big to need more
        cap = r.choice([0, 0, 1, 5])
        put(coaster, 0x98, 'II', cap, BUF if cap or r.random() < 0.5 else 0)
        regions[BUF] = bytearray(r.bytes(0x38 * 6))
    return Call(CALLS[op], [COASTER], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
