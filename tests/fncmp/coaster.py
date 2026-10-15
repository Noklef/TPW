# src/game/coaster: ratings, track spline, orientation and frame against the originals.
# usage: coaster.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x43c000, 0x43c3d0), (0x43dbf0, 0x43ea90), (0x46f210, 0x46f240), (0x470940, 0x470990), (0x43f030, 0x43f150), (0x43fcd0, 0x43fd60), (0x43bc20, 0x43bd20), (0x43b750, 0x43b8b0), (0x43a970, 0x43aa10), (0x43bdb0, 0x43be10), (0x43bfb0, 0x43c000), (0x43b050, 0x43b480), (0x440ad0, 0x440c10)], sines=256)
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


def nan_pair(r, buf, offsets, p=0.25):
    """Two NaNs alike but for the sign, which an add or multiply picks between by operand order."""
    if r.random() < p:
        for o in r.sample(list(offsets), 2):
            put(buf, o, 'I', r.choice([0x7fc00000, 0xffc00000]))
    return buf


def helpers(r, op):
    if op in ('Dot', 'Normalize'):
        a, b = (r.floats(r.bytes(12), range(0, 12, 4)) for _ in range(2))
        if r.random() < 0.2:  # tiny or huge: the registers' range shows
            put(a, 0, 'fff', *(r.choice([1e-30, 3e-25, 1e25, 2e30, 3e38, 1e-39, 1e-42]) * r.uniform(-1, 1) for _ in range(3)))
        nan_pair(r, a, range(0, 12, 4))
        if op == 'Dot':
            ab = nan_pair(r, a + b, range(0, 24, 4))
            return Call(0x43a970, [OUT, OUT + 12], {OUT: ab}, ret='d')
        return Call(0x43a9a0, [OUT], {OUT: a})
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
    if op == 'Extent':
        count = r.randrange(0, 6)
        car, segs, types = r.bytes(0x100), r.bytes(14 * 0x60), r.bytes(0x400)
        put(car, 8, 'II', r.randrange(0, 6), count)
        put(car, 0x28, 'I', SPL + 0x60)  # no segments reads the one before
        put(car, 0x74, 'I', r.f())
        for k in range(14):
            put(segs, k * 0x60 + 4, 'I', NODES + r.randrange(8) * 0x80)
            put(segs, k * 0x60 + 0x38, 'I', r.f())
        for k in range(8):
            put(types, k * 0x80 + 0x24, 'II', r.f(), r.f())
        put(c, 0xac, 'I', r.f())
        if r.random() < 0.3:  # sign-pair NaNs in the front sum: its offset, position, length
            nan = [0x7fc00000, 0xffc00000]
            r.shuffle(nan)
            front = r.choice([0, 1])
            sel = r.sample([0, 1, 2], 2)
            for i, n in zip(sel, nan):
                if i == 0:
                    put(segs, front * 0x60 + 0x60 + 0x38, 'I', n)
                elif i == 1:
                    put(car, 0x74, 'I', n)
                else:
                    put(types, 0x24, 'I', n)
                    put(segs, front * 0x60 + 0x60 + 4, 'I', NODES)
            put(car, 8, 'I', front)
        return Call(0x43bdb0, [COAST, CARS], {COAST: c, CARS: car, SPL: segs, NODES: types})
    if op == 'Drop':
        put(t, 0x24, 'III', r.f(), r.f(), r.f())
        y = r.f()
        args = [r.f(), r.f(), y, r.choice([y, y ^ 0x80000000, r.f(), r.f()])]
        if r.random() < 0.2:  # no drop and no speed: +0 against a -0 minimum
            args = [r.choice([0, 0x80000000]), r.choice([0, 0x80000000]), y, y]
        return Call(0x43bfb0, [TYPE] + args, {TYPE: t}, ret='d')
    if op == 'Window':
        cs, vals = r.bytes(0x300), r.bytes(0x300)
        for k in range(3):  # car k: +0x64 the next car, +0x90 -> +4 -> +8 its value
            put(cs, k * 0x80 + 0x64, 'I', CARS + (k + 1) % 3 * 0x80)
            put(cs, k * 0x80 + 0x90, 'I', SPL + k * 0x100)
            put(vals, k * 0x100 + 4, 'I', SPL + k * 0x100 + 0x80)
            put(vals, k * 0x100 + 0x88, 'I', r.f())
        put(c, 0x54, 'I', CARS)
        put(t, 0x31c, 'I', r.f())
        if r.random() < 0.3:  # NaNs alike but for the sign: which one an add keeps shows
            for o in r.sample([0x88, 0x288, -1], 2):
                nan = r.choice([0x7fc00000, 0xffc00000])
                put(vals, o, 'I', nan) if o >= 0 else put(t, 0x31c, 'I', nan)
        return Call(0x43b750, [COAST], {COAST: c, TYPE: t, CARS: cs, SPL: vals})
    if op == 'InWindow':
        count, start = r.randrange(0, 10), r.randrange(0, 4)
        car, segs, types = r.bytes(0x100), r.bytes(14 * 0x60), r.bytes(0x400)
        put(car, 8, 'II', start, count)
        put(car, 0x28, 'I', SPL)
        mode = r.choice(['exact', 'exact', 'tiny', 'wide'])
        scale = {'exact': r.choice([1.0, 2.0, 0.5]), 'tiny': r.uniform(3, 9), 'wide': r.uniform(0.25, 4)}[mode]
        pos, ty = [], []
        f32 = lambda v: struct.unpack('<f', struct.pack('<f', v))[0]  # noqa: E731
        for k in range(8):
            if mode == 'exact':
                v = (r.randrange(6) / 2, r.randrange(6) / 2)
            elif mode == 'tiny':  # a quotient below float's range: the register keeps its bits
                v = (r.uniform(1e-38, 4e-38), r.uniform(1e-38, 4e-38))
            else:
                v = (r.uniform(0, 3), r.uniform(0, 3))
            ty.append((f32(v[0]), f32(v[1])))
            put(types, k * 0x80 + 0x24, 'ff', *ty[-1])
        for k in range(14):
            ti = r.randrange(8)
            x = {'exact': r.randrange(-16, 16) / 2, 'tiny': 0.0, 'wide': r.uniform(-8, 8)}[mode]
            pos.append((x, ti))
            put(segs, k * 0x60 + 4, 'I', NODES + ti * 0x80)
            put(segs, k * 0x60 + 0x40, 'f', x)
        x, ti = r.choice(pos)
        lo = r.choice([f32(x + ty[ti][0] / scale), r.uniform(-8, 8)])
        x, ti = r.choice(pos)
        hi = r.choice([f32(x - ty[ti][1] / scale), lo + r.uniform(-1, 6)])
        put(c, 0xf8, 'ff', lo, hi)
        put(c, 0xac, 'f', scale)
        if r.random() < 0.15:
            buf, off = r.choice([(segs, r.randrange(14) * 0x60 + 0x40), (types, r.randrange(8) * 0x80 + r.choice([0x24, 0x28])),
                                 (c, r.choice([0xac, 0xf8, 0xfc]))])
            put(buf, off, 'I', r.f())
        return Call(0x43b7e0, [COAST, CARS, OUT, OUT + 4], {COAST: c, CARS: car, SPL: segs, NODES: types, OUT: r.bytes(8)})
    if op == 'Segs':
        count = r.randrange(0, 13)
        car, segs = r.bytes(0x100), r.bytes(14 * 0x60)
        put(car, 4, 'I', TYPE)
        put(car, 0xc, 'I', count)
        put(car, 0x28, 'I', SPL)
        put(car, 0x2c, 'I', r.choice([(count - 1) // 2 if count else 0, r.randrange(max(count, 1)), 0, count]))
        for k in range(14):
            put(segs, k * 0x60 + 8, 'I', r.choice([k, k, 0, (count - 1) & 0xffffffff, r.getrandbits(32)]))
        for o in (0x1c, 0x20, 0x24):
            put(t, o, 'I', r.f())
        if r.random() < 0.2:  # two NaNs alike but for the sign: the sum keeps its own (dst wins)
            put(t, r.choice([0x1c, 0x20, 0x24]), 'I', 0x7fc00000)
            put(t, r.choice([0x1c, 0x20, 0x24]), 'I', 0xffc00000)
        put(c, 0xac, 'I', r.f())
        return Call(0x43bc20, [COAST, CARS], {COAST: c, TYPE: t, CARS: car, SPL: segs})
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



def keys(r, op):
    start = r.choice([0.0, r.uniform(-5, 5), r.uniform(0, 100)])
    span = r.choice([r.uniform(0, 20), r.uniform(-5, 5), 0.0, 1.0])
    n = r.choice([0, 1, 2, 3, r.randrange(2, 20)])
    same = r.random() < 0.6
    b = put(r.bytes(0x100), 0x90, 'I', TYPE)
    ks, t = r.bytes(64 * 0x10), start - r.uniform(0, 3) * r.randrange(3)
    for k in range(63):  # rising, with repeats and exact sample times
        t += r.choice([0, r.uniform(0, 1), r.uniform(0, 4)])
        v = r.choice([t, t, start + span * r.randrange(max(n, 1)) / max(n - 1, 1)])
        put(ks, k * 0x10, 'f', v)
    put(ks, 63 * 0x10, 'f', 1e30 if r.random() < 0.9 else float('inf'))
    regs = {COAST: r.bytes(0x100), SPL: b, TYPE: put(bytearray(4), 0, 'I', n), NODES: ks, OUT: r.bytes(0x600),
            0x79240c: put(bytearray(4), 0, 'f', start)}
    return Call(0x43f030, [SPL if same else COAST, SPL, NODES, OUT, fb(span)], regs, ret='a')



def lists(r, op):
    g, r1, r2, mesh, m2, slots, pool, lst, tail = (r.bytes(0x100) for _ in range(9))
    put(g, 0x38, 'I', TYPE)
    put(r1, 0x24, 'I', NODES)
    put(r2, 4, 'I', FORCE)
    put(mesh, 0x78, 'I', CARS)
    put(m2, 0x6c, 'I', SPL)
    put(lst, 8, 'I', r.choice([0, PIECES]))
    put(lst, 0xc, 'I', r.choice([0, YAW]))
    put(lst, 0x10, 'I', r.choice([0, 0, r.getrandbits(32)]))
    regs = {COAST: g, TYPE: r1, NODES: r2, FORCE: mesh, CARS: m2, SPL: slots, ORIENT: pool,
            OUT: lst, YAW: tail, 0x792218: put(bytearray(4), 0, 'I', COAST),
            0x79c604: put(bytearray(4), 0, 'I', ORIENT + 0x10 * r.randrange(8)),
            0x7925c8: put(bytearray(4), 0, 'I', r.randrange(12))}
    return Call(0x43fcd0, [OUT], regs, ret='a')


def ring(r, c, off, buf):
    """A ring at c+off over buf: capacity 1..8, pointers on its slots, small counts."""
    cap = r.randrange(1, 9)
    slot = lambda: buf + 4 * r.choice([r.randrange(cap), r.randrange(cap), cap - 1, 0, r.randrange(-4, 0) if r.random() < 0.2 else 0])  # noqa: E731
    head = slot()
    put(c, off, 'IIIIIIII', cap, buf, r.randrange(-2, 10) & 0xffffffff, r.randrange(0, cap + 1), r.choice([0, 0, r.randrange(4)]),
        head, slot(), r.choice([0, head, slot()]))


def queue(r, op):
    if op == 'Link':
        nodes, n = r.bytes(0x300), r.randrange(0, 8)
        keys = [r.randrange(6) for _ in range(n)]
        for k in range(n):
            put(nodes, k * 0x30 + 0x14, 'II', r.getrandbits(32), NODES + (k + 1) * 0x30 if k + 1 < n else 0)
            put(nodes, k * 0x30 + 0x24, 'I', keys[k])
        regs = {NODES: nodes, 0x790fe0: put(bytearray(4), 0, 'I', NODES if n else 0)}
        return Call(0x43b050, [r.randrange(6)], regs, ret='a')
    c, index = r.bytes(0x200), r.randrange(4)
    ring(r, c, 0xb0, PIECES + 0x20)  # pointers may sit up to 4 slots before a buffer
    ring(r, c, 0xd0, PIECES + 0x60)
    regs = {COAST: c, PIECES: r.bytes(0x100), 0x790bd0: put(r.bytes(16), index * 4, 'I', COAST)}
    if op == 'QPush':
        return Call(0x43b0e0, [index, r.getrandbits(32)], regs)
    if op == 'QAdd':
        return Call(0x43b130, [COAST, r.getrandbits(32)], regs)
    if op == 'QRoom':
        return Call(0x43b1f0, [index], regs, ret='a')
    if op == 'QPop':
        return Call(0x43b220, [index], regs, ret='a')
    if r.random() < 0.3:  # zero peeps are taken but not passed on
        put(regs[PIECES], 0, 'IIII', 0, 0, r.getrandbits(32), 0)
    return Call(0x43b390, [COAST], regs)


def cursor(r, op):
    """Three pieces in a ring (+0x64 on, +0x68 back), each with a node list {count, nodes}."""
    pieces, lists, nodes = r.bytes(0x300), r.bytes(0x30), r.bytes(3 * 4 * 0x58)
    counts = [r.choice([1, 1, 2, 3, 4, 0]) for _ in range(3)]
    for k in range(3):
        put(pieces, k * 0x100 + 0x64, 'II', NODES + (k + 1) % 3 * 0x100, NODES + (k + 2) % 3 * 0x100)
        put(pieces, k * 0x100 + 0x90, 'I', SPL + k * 0x10)
        put(lists, k * 0x10, 'II', counts[k], FORCE + k * 4 * 0x58)
    r.floats(nodes, range(0, len(nodes), 4))
    a = r.randrange(3)
    b = r.choice([a, a, (a + 1) % 3, r.randrange(3)])
    ia = r.choice([max(counts[a] - 1, 0), r.randrange(4), 0, 0xffffffff])
    ib = r.choice([(ia + 1) & 0xffffffff, 0, r.randrange(4)]) if a == b else r.choice([0, r.randrange(4)])
    cur = put(bytearray(16), 0, 'IIII', ia, NODES + a * 0x100, min(ib, 3), NODES + b * 0x100)
    regs = {NODES: pieces, SPL: lists, FORCE: nodes, OUT: cur}
    if op == 'CNext':
        return Call(0x440ad0, [OUT], regs)
    if op == 'CPrev':
        return Call(0x440b50, [OUT], regs)
    put(cur, 0, 'I', min(ia, 3))
    regs[YAW] = r.bytes(12)
    return Call(0x440bc0, [OUT, r.choice([fb(r.random()), fb(0.0), fb(1.0), r.f()]), YAW], regs)


KINDS = {'Reset': ratings, 'Step': ratings, 'Finish': ratings, 'Point': spline, 'Tangent': spline, 'Set': orient,
         'Init': orient, 'Frame': frame, 'At': frame,
         'RoundUp': helpers, 'Lerp': helpers, 'Dot': helpers, 'Normalize': helpers, 'Swing': cars, 'Space': cars, 'Segs': cars, 'Extent': cars, 'Drop': cars, 'Window': cars, 'InWindow': cars, 'Dist': cars,
         'NodeLerp': nodes, 'PastEnd': nodes, 'Zone': nodes, 'Merge': keys, 'Append': lists,
         'Link': queue, 'QPush': queue, 'QAdd': queue, 'QRoom': queue, 'QPop': queue, 'QAdvance': queue,
         'CNext': cursor, 'CPrev': cursor, 'CPoint': cursor}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    return KINDS[op](r, op)


if __name__ == '__main__':
    B.main(case)
