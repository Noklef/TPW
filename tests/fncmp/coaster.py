# src/game/coaster: ratings, track spline, orientation and frame against the originals.
# usage: coaster.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x43c000, 0x43c3d0), (0x43dbf0, 0x43ea90), (0x46f210, 0x46f240), (0x470940, 0x470990)], sines=256)
G = 0x7923a8  # CoasterRatingState
COAST, TYPE, NODES, FORCE, CARS, SPL, PIECES, OUT, ORIENT, FLAGS, YAW = (RAM + k * 0x1000 for k in range(11))
RING = 10


def spline_nodes(r, count, ring=False):
    spl, pieces = r.bytes(count * 0x60), r.bytes(count * 0x100)
    for k in range(count):
        b = k * 0x60
        r.floats(spl, range(b + 0x34, b + 0x58, 4))
        if ring:  # two rings of rising distances
            j = k % RING
            put(spl, b + 8, 'f', j + r.choice([0, 0, 0.5, r.random()]) if j else 0.0)
            put(spl, b + 0x18, 'I', SPL + (k - j + (j + 1) % RING) * 0x60)
        else:
            put(spl, b + 8, 'f', r.choice([r.uniform(0, 7.99), r.randrange(8), -0.5]))
        put(spl, b + 0xc, 'I', r.f(5))
        put(spl, b + 0x10, 'I', PIECES + k * 0x100)
        put(pieces, k * 0x100, 'I', r.choice([0, 1, 5, r.randrange(100), r.getrandbits(32)]))
        put(pieces, k * 0x100 + 0x6c, 'I', r.f(5))
        put(pieces, k * 0x100 + 0xe4, 'I', r.f(4))
    return spl, pieces


def orientation(r):
    o = r.bytes(0x20)
    return put(o, 0, 'I', r.choice([0, 1, r.getrandbits(32)]))


def ratings(r, op):
    g = r.floats(r.bytes(0x38), range(0, 0x38, 4))
    for off in (4, 0x1c):
        put(g, off, 'I', r.choice([0, 0, 1, 1, r.getrandbits(32)]))
    if r.random() < 0.5:
        put(g, 0x20, '3I4xI', 0, 0, 0, 0)  # as the game leaves them
    t = r.bytes(0x400)
    for off in range(0x320, 0x35c, 4):
        put(t, off, 'I', r.f(2) if r.random() < 0.7 else r.f())
    for off in (0x34c, 0x350, 0x354, 0x358):
        if r.random() < 0.6:
            put(t, off, 'I', r.pos())
    c, cars, n = r.bytes(0x200), r.bytes(8 * 0x80), r.randrange(0, 9)
    for k in range(8):
        put(cars, k * 0x80 + 8, 'I', r.choice([0x40, 0x80, 0xc0, 0xc0, 0, r.getrandbits(32)]))
        put(cars, k * 0x80 + 0x64, 'I', CARS + (k + 1) * 0x80 if k + 1 < n else 0)
    put(c, 4, 'I', TYPE)
    put(c, 0x9c, 'I', NODES)
    put(c, 0x54, 'I', CARS if n else 0)
    put(c, 0xa8, 'II', r.f(), r.f())
    regs = {G: g, TYPE: t, COAST: c, CARS: cars, NODES: r.bytes(0x38 * 12), FORCE: put(bytearray(12), 0, '3I', r.f(4), r.f(4), r.f(4))}
    if op == 'Reset':
        return Call(0x43dbf0, [COAST], regs)
    if op == 'Step':
        return Call(0x43dc50, [COAST, r.choice([0, 1, 2, 3, 4, 5, 11, r.randrange(12)]), r.getrandbits(32), r.f(30), r.f(4),
                               FORCE, r.f()], regs)
    return Call(0x43df90, [COAST, r.f(), r.f(), r.f()], regs)


def spline(r, op):
    spl, pieces = spline_nodes(r, 2)
    t = r.choice([fb(r.random()), fb(0.0), fb(1.0), fb(0.5), r.f()])
    return Call(0x43e1b0 if op == 'Point' else 0x43e320, [SPL, SPL + 0x60, t, OUT], {SPL: spl, OUT: r.bytes(12)})


def orient(r, op):
    v = [r.f(), r.f(), r.f()]
    regs = {ORIENT: orientation(r)}
    yaw = 0
    if r.random() < 0.4:
        regs[YAW] = put(bytearray(4), 0, 'I', r.choice([r.f(4), r.f(), 0x7f800001, 0xff812345, fb(3.14159274)]))
        yaw = YAW
    if op == 'Set':
        return Call(0x43e590, v + [r.getrandbits(32), ORIENT, yaw], regs)
    regs[SPL] = put(bytearray(12), 0, '3I', *v)
    return Call(0x43e540, [r.getrandbits(32), SPL, r.getrandbits(32), ORIENT, r.getrandbits(32), yaw], regs)


def frame(r, op):
    nodes = r.bytes(12 * 0x38)
    for k in range(12):
        put(nodes, k * 0x38 + 0x18, 'I', r.f(4))
    flags = r.choice([0, 8, r.getrandbits(8)])
    spl, pieces = spline_nodes(r, 4 if op == 'Frame' else 2 * RING, ring=op == 'At')
    regs = {SPL: spl, PIECES: pieces, OUT: r.bytes(12), ORIENT: orientation(r)}
    o = 0 if r.random() < 0.05 else ORIENT
    if op == 'Frame':
        regs[NODES], regs[FLAGS] = nodes, bytearray([flags, 0, 0, 0])
        t = r.choice([fb(r.random()), fb(0.0), fb(1.0), fb(0.5), r.f()])
        return Call(0x43e6b0, [NODES, FLAGS, SPL, SPL + 0x60, SPL + 0xc0, SPL + 0x120, t, OUT, o], regs)
    pos = r.choice([r.uniform(0, RING - 1.01), float(r.randrange(RING - 1)), 0.0])
    for k in range(12):  # each coaster node's spline nodes, up to 3 behind its own
        s = min(max(0, k - r.randrange(4)), RING - 2)
        put(nodes, k * 0x38 + 0x2c, 'II', SPL + s * 0x60, SPL + (RING + s) * 0x60)
    c = r.bytes(0x100)
    put(c, 0x9c, 'I', NODES)
    c[0x3c] = flags
    regs[NODES], regs[COAST] = nodes, c
    return Call(0x43c0a0, [COAST, fb(pos), OUT, o], regs)


def helpers(r, op):
    if op == 'RoundUp':
        x = r.choice([r.f(), fb(r.randrange(-50, 50) + r.choice([0, 0, 0.5, r.random()])), fb(-0.25), fb(2 ** 40)])
        return Call(0x43e160, [x], {}, ret='a')
    a, b = (r.choice([r.f(4), r.f(), fb(r.uniform(-3.2, 3.2)), fb(r.uniform(-7, 7))]) for _ in range(2))
    t = r.choice([fb(r.random()), fb(0.0), fb(1.0), r.f()])
    return Call(0x43e480, [a, b, t], {}, ret='d')


def cars(r, op):
    c, t = r.bytes(0x200), r.bytes(0x400)
    put(c, 4, 'I', TYPE)
    if op == 'Swing':
        car = r.floats(r.bytes(0x100), range(0x4c, 0x58, 4))
        put(car, 0x94, 'I', r.choice([0, 0x80000000, r.f(4), r.f()]))
        put(car, 8, 'I', r.choice([0, 0x100, r.getrandbits(32)]))
        put(car, 0x64, 'II', CARS, CARS + 0x80)
        put(t, 0x10, 'I', r.f())
        return Call(0x43c160, [COAST, OUT, r.f()], {COAST: c, TYPE: t, OUT: car, CARS: r.floats(r.bytes(0x100), range(0, 0x100, 4))},
                    ret='d')
    if op == 'Dist':
        return Call(0x470940, [SPL, SPL + 12], {SPL: r.floats(r.bytes(24), range(0, 24, 4))}, ret='d')
    # SpaceCars: loop-critical values stay finite and positive, or the walk never ends (sic)
    count, n = r.randrange(2, 13), r.randrange(1, 7)
    length = r.choice([float(count), float(count), count - r.random()])
    nodes = r.bytes(16 * 0x38)
    for k in range(16):
        put(nodes, k * 0x38 + 0x1c, 'f', r.uniform(0.5, 4))
    put(t, 0x2ec, 'I', r.choice([0, r.randrange(100, 4000), r.randrange(8000)]))
    put(c, 0x9c, 'I', NODES)
    put(c, 0xa4, 'Iff', count, length, r.uniform(0.5, 4))
    put(c, 0xf4, 'I', n)
    put(c, 0x128, 'I', CARS)
    cs, spl = r.bytes(8 * 0x80), r.bytes(14 * 0x60)
    for k in range(1, 14):
        put(spl, k * 0x60 - 0x20, 'f', r.choice([r.uniform(-length, length), float(r.randrange(count)), 0.0]))
        if n == 1 and r.random() < 0.3:  # one car keeps its seed, so a NaN ends no walk
            put(spl, k * 0x60 - 0x20, 'I', r.choice([0x7fc00000, 0xffc00000, 0x7f800001, 0xff812345]))
    for k in range(8):
        a = r.randrange(0, 13)
        b = r.randrange(1, 14 - a) if a == 0 else r.randrange(0, 14 - a)
        put(cs, k * 0x80 + 8, 'II', a, b)
        put(cs, k * 0x80 + 0x28, 'I', SPL)
    return Call(0x43c1f0, [COAST], {COAST: c, TYPE: t, NODES: nodes, CARS: cs, SPL: spl})



def nodes(r, op):
    c = r.bytes(0x200)
    put(c, 0x9c, 'I', NODES)
    if op == 'NodeLerp':
        ns = r.floats(r.bytes(12 * 0x38), [k * 0x38 + o for k in range(12) for o in (4, 8, 0xc)])
        t = r.choice([fb(r.random()), fb(0.0), fb(1.0), r.f()])
        return Call(0x43c000, [COAST, r.randrange(11), t, OUT], {COAST: c, NODES: ns, OUT: r.bytes(12)})
    if op == 'PastEnd':
        put(c, 0xa8, 'I', r.f(10))
        x = r.choice([r.f(10), struct.unpack('<I', c[0xa8:0xac])[0]])
        return Call(0x43c080, [COAST, x], {COAST: c}, ret='a')
    count = r.randrange(1, 9)
    boxes = r.bytes(8 * 0x2c)
    for k in range(8):
        for j in range(4):
            lo = r.uniform(-50, 40)
            put(boxes, k * 0x2c + 8 * j, 'ff', lo, lo + r.uniform(0, 30))
            if r.random() < 0.05:
                put(boxes, k * 0x2c + 8 * j + 4 * r.randrange(2), 'I', r.f())
        put(boxes, k * 0x2c + 0x20, 'I', r.choice([0, r.getrandbits(32)]))
    put(c, 0x78, 'II', SPL, count)
    k = r.randrange(8) * 0x2c
    inside = [struct.unpack_from('<f', boxes, k + 8 * j)[0] + r.uniform(0, 1) for j in range(4)]
    if r.random() < 0.3:  # a NaN bound on the box the point is in
        put(boxes, k + 4 * r.randrange(8), 'I', r.choice([0x7fc00000, 0xffc00000]))
    pos = put(bytearray(16), 0, '4I', r.getrandbits(32), *[fb(v) if r.random() < 0.9 else r.f() for v in inside[1:]])
    t = fb(inside[0]) if r.random() < 0.9 else r.f()
    return Call(0x43e9c0, [COAST, r.getrandbits(32), OUT, t], {COAST: c, SPL: boxes, OUT: pos}, ret='a')


KINDS = {'Reset': ratings, 'Step': ratings, 'Finish': ratings, 'Point': spline, 'Tangent': spline, 'Set': orient,
         'Init': orient, 'Frame': frame, 'At': frame,
         'RoundUp': helpers, 'Lerp': helpers, 'Swing': cars, 'Space': cars, 'Dist': cars,
         'NodeLerp': nodes, 'PastEnd': nodes, 'Zone': nodes}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    return KINDS[op](r, op)


if __name__ == '__main__':
    B.main(case)
