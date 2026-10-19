# The track rides' collision object pool and list releases (src/game/world/collobj: 0x544330,
# 0x5457e0, 0x545810, 0x54b290, 0x549070) against the originals: lists empty to 8 nodes in any order in memory,
# masks 0, one bit or any, nodes in use or not; chunk lists of up to three chunks with a node pair or none. The nearest area node
# (0x549070): see nearest().
# usage: collobj.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

ALL = ['Reset', 'ReleaseC0', 'KeepA8', 'FreeAll', 'Nearest']
OPS = [k for k in os.environ.get('OPS', ','.join(ALL)).split(',') if k]

B = Bench([(0x544330, 0x544356), (0x5457e0, 0x545809), (0x545810, 0x545888), (0x54b290, 0x54b2f0), (0x549070, 0x5491db),
           (0x5491e0, 0x5493ef), (0x5493f0, 0x5494c6), (0x549020, 0x54906e), (0x67a830, 0x67a860)])
NODES, POOL, GLOBALS = RAM + 0x200, RAM + 0x1000, 0x877b70


def free_all(r):
    # chunks at RAM + 0x100 + 0x40 * i (list head at the owner's +0xbc), their nodes at RAM + 0x400 + 0x20 * j
    obj = bytearray(r.bytes(0x400)) + bytes(0x200)
    owner = RAM + 0x20
    nodes = [RAM + 0x400 + 0x20 * j for j in range(6)]
    cs = [RAM + 0x100 + 0x40 * i for i in range(3)]
    put(obj, 0x20 + 0xbc, 'I', cs[0] if r.random() < 0.9 else 0)
    for i, c in enumerate(cs):
        put(obj, 0x100 + 0x40 * i + 0x24, 'I', cs[i + 1] if i + 1 < len(cs) and r.random() < 0.8 else 0)
        put(obj, 0x100 + 0x40 * i + 0x1c, 'I', r.choice([0, nodes[2 * i]]))
        put(obj, 0x100 + 0x40 * i, 'I', r.getrandbits(32))
    for j in range(6):
        put(obj, 0x400 + 0x20 * j + 0x14, 'I', nodes[j + 1] if j % 2 == 0 and r.random() < 0.7 else 0)
        put(obj, 0x400 + 0x20 * j, 'I', r.getrandbits(32))
    return Call(0x54b290, [owner], {RAM: obj, 0x877b70: r.bytes(0x20)})


def nearest(r):
    """0x549070: up to 10 area nodes {flags, centre x, y, radii} at NODES (bit 0 mostly set, blockers 0x40 now and then, the side bits all
    four, some or any, now and then the box flag 0x20), centres on a 0x400 grid (so the 0x87c moves and the 0x1800 diagonal meet) or
    anywhere near; the count the nodes in use, now and then fewer, 0 or no table; the area one of the nodes (self 0 or 1) or apart."""
    n = r.randrange(11)
    nodes = bytearray(r.bytes(0x20 * 11))
    live = 0
    for i in range(n):
        if r.random() < 0.15:
            flags = r.getrandbits(32) & ~1
        else:
            sides = 0x1e if r.random() < 0.5 else sum(b for b in (2, 4, 8, 0x10) if r.random() < 0.6)
            flags = 1 | sides | (0x40 if r.random() < 0.25 else 0) | (0x20 if r.random() < 0.2 else 0) | (r.getrandbits(32) & ~0x7f if r.random() < 0.1 else 0)
        live += flags & 1
        grid = lambda: r.choice([r.randrange(-6, 7) * 0x400, r.randrange(-6, 7) * 0x400 + r.choice([0x87c, -0x87c]), r.randrange(-0x2000, 0x2000)])
        put(nodes, 0x20 * i, 'IiiII', flags, grid(), grid(), r.choice([0, 0x400, 0x800, 0x87c, 0x1000, r.randrange(0x2000)]), r.choice([0, 0x400, 0x900, r.randrange(0x2000)]))
    count = r.choice([live, live, live, live, max(live - 1, 0), 0])
    other = RAM + 0x100
    area = bytearray(r.bytes(0x20))
    put(area, 0, 'IiiII', r.choice([0x1f, 0x1e, 3, 0x19, r.getrandbits(32)]), r.randrange(-6, 7) * 0x400, r.randrange(-6, 7) * 0x400, r.choice([0x400, 0x800, r.randrange(0x2000)]), r.randrange(0x2000))
    globals_ = bytearray(r.bytes(0x10))
    put(globals_, 0, 'II', 0 if r.random() < 0.03 else NODES, count)
    target = NODES + 0x20 * r.randrange(n) if n and r.random() < 0.6 else other
    point = lambda: r.choice([r.randrange(-0x2000, 0x2000), r.randrange(-6, 7) * 0x400, r.getrandbits(32) - (1 << 31)]) & 0xffffffff
    return Call(0x549070, [target, point(), point(), r.choice([0, 1, r.getrandbits(32)])], {NODES: nodes, other: area, GLOBALS: globals_}, ret='a')


def case(r):
    op = ALL.index(r.deal(OPS))
    if op == 4:
        return nearest(r)
    if op == 3:
        return free_all(r)
    globals_ = bytearray(r.bytes(0x20))
    if op == 0:
        put(globals_, 0x18, 'I', POOL)
        return Call(0x544330, [], {GLOBALS: globals_, POOL: r.bytes(0x5000)})
    collider = bytearray(r.bytes(0x100))
    put(collider, 0xac, 'I', r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    put(globals_, 0xc, 'I', r.choice([0, 1, 0x10000, r.getrandbits(32)]))
    nodes = bytearray(r.bytes(8 * 0x18))
    n = r.choice([0, 1, 2, r.randrange(9)])
    order = r.sample(range(8), n)
    for k, i in enumerate(order):
        put(nodes, i * 0x18 + 0x14, 'I', NODES + order[k + 1] * 0x18 if k + 1 < n else 0)
        put(nodes, i * 0x18, 'I', r.choice([0, 1, 2, 3, 0x10, 0x11, r.getrandbits(32)]))
    put(collider, 0xc0 if op == 1 else 0xa8, 'I', NODES + order[0] * 0x18 if n else 0)
    args = [RAM]
    if op == 2:
        args.append(r.choice([0, 1, 2, 0x10, 0xfffffffe, r.getrandbits(32)]))
    return Call(0x5457e0 if op == 1 else 0x545810, args, {RAM: collider, NODES: nodes, GLOBALS: globals_})


if __name__ == '__main__':
    B.main(case)
