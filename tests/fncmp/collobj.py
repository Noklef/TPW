# The track rides' collision object pool and list releases (src/game/world/collobj: 0x544330,
# 0x5457e0, 0x545810, 0x54b290, 0x549070) against the originals: lists empty to 8 nodes in any order in memory,
# masks 0, one bit or any, nodes in use or not; chunk lists of up to three chunks with a node pair or none. The nearest area node
# (0x549070): see nearest(); a ride car steered (0x547c60): see steer(); bounced off the blockers (0x5494d0): see bounce(); kept in its
# area (0x5497b0): see keep().
# usage: collobj.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

ALL = ['Reset', 'ReleaseC0', 'KeepA8', 'FreeAll', 'Nearest', 'Steer', 'Bounce', 'Keep']
OPS = [k for k in os.environ.get('OPS', ','.join(ALL)).split(',') if k]

B = Bench([(0x544330, 0x544356), (0x5457e0, 0x545809), (0x545810, 0x545888), (0x54b290, 0x54b2f0), (0x549070, 0x5491db),
           (0x5491e0, 0x5493ef), (0x5493f0, 0x5494c6), (0x549020, 0x54906e), (0x67a830, 0x67a860),
           (0x547c60, 0x547f4d), (0x5da3c0, 0x5da3c1), (0x5494d0, 0x5497a9), (0x5497b0, 0x549b4d)])
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
    anywhere near, the blockers now and then on another's moved centre's edge; the count the nodes in use, now and then fewer, 0 or no table; the area one of the nodes (self 0 or 1) or apart."""
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
    # blockers set on another node's moved centre's edge: its radius (half width) or one past it away along x or y
    for i in range(n):
        flags = struct.unpack_from('<I', nodes, 0x20 * i)[0]
        others = [j for j in range(n) if j != i and struct.unpack_from('<I', nodes, 0x20 * j)[0] & 0x41 == 1]
        if flags & 0x41 != 0x41 or not others or r.random() < 0.3:
            continue
        f, x, y = struct.unpack_from('<Iii', nodes, 0x20 * r.choice(others))
        y += 0 if f & 0x18 == 0x18 else -0x87c if not f & 8 else 0x87c
        x += 0 if f & 6 == 6 else 0x87c if not f & 2 else -0x87c
        k = r.choice([0, 0x400, 0x87c, r.randrange(0x1000)])
        d = r.choice([k, k + 1, -k, -k - 1])
        x, y = (x + d, y) if r.random() < 0.5 else (x, y + d)
        put(nodes, 0x20 * i, 'IiiII', 0x5f | (flags & 0x20), x, y, k, k)
    count = r.choice([live, live, live, live, max(live - 1, 0), 0])
    other = RAM + 0x100
    area = bytearray(r.bytes(0x20))
    put(area, 0, 'IiiII', r.choice([0x1f, 0x1e, 3, 0x19, r.getrandbits(32)]), r.randrange(-6, 7) * 0x400, r.randrange(-6, 7) * 0x400, r.choice([0x400, 0x800, r.randrange(0x2000)]), r.randrange(0x2000))
    globals_ = bytearray(r.bytes(0x10))
    put(globals_, 0, 'II', 0 if r.random() < 0.03 else NODES, count)
    target = NODES + 0x20 * r.randrange(n) if n and r.random() < 0.6 else other
    point = lambda: r.choice([r.randrange(-0x2000, 0x2000), r.randrange(-6, 7) * 0x400, r.getrandbits(32) - (1 << 31)]) & 0xffffffff
    return Call(0x549070, [target, point(), point(), r.choice([0, 1, r.getrandbits(32)])], {NODES: nodes, other: area, GLOBALS: globals_}, ret='a')


def steer(r):
    """0x547c60: nearest()'s nodes and area, the car at CAR with flags 0x10000 / 0x4000 / 0x80000 mixed, its area that one or none, its
    position and target on the same grid or anywhere, the kind (KIND) a small or any speed and turn rate, +0x54 2 now and then; the sine
    table small values or any; the point as nearest()'s. Now and then the car goes straight to a target at a distance's edge (see below)
    with its heading on or about the bearing."""
    c = nearest(r)
    regions = dict(c.regions)
    area, x, y = c.args[0], c.args[1], c.args[2]
    car = bytearray(r.bytes(0xa0))
    flags = r.getrandbits(32) if r.random() < 0.1 else sum(b for b in (0x10000, 0x4000, 0x80000, 0x20000) if r.random() < 0.5)
    spot = lambda: r.choice([r.randrange(-6, 7) * 0x400, r.randrange(-0x3000, 0x3000), r.getrandbits(32) - (1 << 31), r.randrange(-0x30000, 0x30000)])
    put(car, 0, 'I', flags)
    put(car, 0x34, 'ii', spot(), spot())
    put(car, 0x6c, 'ii', spot(), spot())
    put(car, 0x74, 'ii', r.choice([0, r.randrange(-0x400, 0x400)]), r.choice([0, r.randrange(-0x400, 0x400)]))
    put(car, 0x50, 'I', r.choice([r.randrange(0x200), r.getrandbits(32)]))
    put(car, 0x68, 'i', r.choice([0, r.randrange(-8, 40), r.randrange(-90, -20), r.getrandbits(32) - (1 << 31)]))
    put(car, 0x98, 'II', area if r.random() < 0.85 else 0, KIND)
    kind = bytearray(r.bytes(0x60))
    put(kind, 0x20, 'i', r.choice([r.randrange(0, 80), r.getrandbits(32) - (1 << 31)]))
    put(kind, 0x28, 'i', r.choice([r.randrange(0, 0x40), r.randrange(-0x40, 0x200), -1, -3, r.getrandbits(32) - (1 << 31)]))
    put(kind, 0x54, 'i', r.choice([2, 0, 1, 3]))
    sine = bytearray(r.bytes(0x800))
    if r.random() < 0.8:
        for i in range(0x200):
            put(sine, 4 * i, 'i', r.randrange(-1024, 1025))
    if r.random() < 0.4:   # (straight to a target: the distance's edges (|dx| 0x8000, sums one below a square), none, and the heading about the bearing)
        put(car, 0, 'I', flags & ~0x10000)
        j = r.randrange(3, 150)
        dx, dy = r.choice([(2 * j * j, 2 * j), (0, 0), (0x8000, r.randrange(-0x7fff, 0x8000)), (-0x8000, r.randrange(0x100)), (0x7fff, 0x8000),
                           (r.randrange(-50, 50), -(1 << 31)), (r.randrange(-0x2000, 0x2000), r.randrange(-0x2000, 0x2000))])
        if r.random() < 0.5:
            dx, dy = dy, dx
        dx, dy = r.choice([1, -1]) * dx, r.choice([1, -1]) * dy
        px, py = struct.unpack_from('<ii', car, 0x34)
        put(car, 0x6c, 'II', (px + dx) & 0xffffffff, (py + dy) & 0xffffffff)
        ndy = -dy if -dy < (1 << 31) else dy
        bearing = int(math.atan2(dx, ndy) / math.pi * 256 - 256)
        rate = struct.unpack_from('<i', kind, 0x28)[0]
        put(car, 0x50, 'I', (-bearing + r.choice([0, 0, 1, -1, 2, rate // 2, -(rate // 2), rate // 2 + 1, 0x16, 0x15, 0x30, 0x2f, 0x100, 0x101])) & 0x1ff)
    regions.update({CAR: car, KIND: kind, 0x877358: sine})
    return Call(0x547c60, [CAR, x, y], regions, ret='a')


CAR, KIND = RAM + 0x800, RAM + 0x900


def bounce(r):
    """0x5494d0: steer()'s car, nodes (more of them blockers) and sine table; the car's position in a blocker (on its centre, within its
    radius) now and then; its motion small, zero or any; the kind's +0 one of the spinning kinds (-5, -8, -12, -13) or not, its +0x2c small
    or any. A car on a node's centre divides by 0: a crash in both."""
    c = steer(r)
    regions = dict(c.regions)
    nodes = bytearray(regions[NODES])
    for i in range(11):
        if r.random() < 0.5:
            put(nodes, 0x20 * i, 'I', struct.unpack_from('<I', nodes, 0x20 * i)[0] | 0x41)
    car, kind = bytearray(regions[CAR]), bytearray(regions[KIND])
    if r.random() < 0.7:
        f, x, y, rx, ry = struct.unpack_from('<IiiII', nodes, 0x20 * r.randrange(11))
        reach = max(1, min(rx, ry, 0x1000))
        put(car, 0x34, 'ii', x + r.choice([0, 1, -1, r.randrange(-reach, reach + 1)]), y + r.choice([0, 1, r.randrange(-reach, reach + 1)]))
    put(car, 0x44, 'ii', *(r.choice([0, r.randrange(-0x400, 0x400), r.randrange(-40, 40), r.getrandbits(32) - (1 << 31)]) for _ in range(2)))
    put(car, 0x58, 'i', r.choice([0, r.randrange(-0x2000, 0x2000), r.getrandbits(32) - (1 << 31)]))
    put(kind, 0, 'i', r.choice([-5, -8, -12, -13, -5, 0, -1, r.getrandbits(32) - (1 << 31)]))
    put(kind, 0x2c, 'i', r.choice([0, r.randrange(-0x400, 0x400), r.getrandbits(32) - (1 << 31)]))
    regions.update({NODES: nodes, CAR: car, KIND: kind})
    return Call(0x5494d0, [CAR], regions)


def keep(r):
    """0x5497b0: bounce()'s car, nodes, kind and table; its area (nearest()'s, a box half the time) or none; its position about the area's
    edges (inside, on, one or a few past, or far), its speed small or any, its counter +0xa4 negative, 0 or the largest."""
    c = bounce(r)
    regions = dict(c.regions)
    car = bytearray(regions[CAR]) + r.bytes(8)
    area = struct.unpack_from('<I', car, 0x98)[0]
    if area:
        key = next(k for k in regions if k <= area < k + len(regions[k]))
        block = bytearray(regions[key])
        o = area - key
        if r.random() < 0.5:
            put(block, o, 'I', struct.unpack_from('<I', block, o)[0] ^ 0x20)
        regions[key] = block
        f, x, y, rx, ry = struct.unpack_from('<IiiII', block, o)
        if r.random() < 0.8:
            edge = lambda h: r.choice([0, 1, -1, 2, 40, -40, r.randrange(-0x800, 0x800)]) + r.choice([h, -h]) if h < 0x10000 else r.randrange(-0x800, 0x800)
            put(car, 0x34, 'II', *((v + edge(h)) & 0xffffffff for v, h in ((x, rx), (y, ry))))
    put(car, 0x3c, 'ii', *(r.choice([0, r.randrange(-0x400, 0x400), r.getrandbits(32) - (1 << 31)]) for _ in range(2)))
    put(car, 0xa4, 'i', r.choice([-1, 0, 5, 0x7fffffff, -(1 << 31)]))
    regions[CAR] = car
    return Call(0x5497b0, [CAR], regions)


def case(r):
    op = ALL.index(r.deal(OPS))
    if op == 5:
        return steer(r)
    if op == 6:
        return bounce(r)
    if op == 7:
        return keep(r)
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
