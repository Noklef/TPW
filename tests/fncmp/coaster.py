# src/game/coaster: ratings, track spline, orientation and frame against the originals.
# usage: coaster.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import math
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x43c000, 0x43c3d0), (0x43dbf0, 0x43ea90), (0x46f210, 0x46f240), (0x470940, 0x470990), (0x43f030, 0x43f150), (0x43fcd0, 0x43fd60), (0x43bc20, 0x43bd20), (0x43b750, 0x43b8b0), (0x43a970, 0x43aa10), (0x43bdb0, 0x43be10), (0x43bfb0, 0x43c000), (0x43b050, 0x43b480), (0x440ad0, 0x440c10), (0x4386d0, 0x438800), (0x43ea90, 0x43eac0), (0x43e0b0, 0x43e0e0), (0x43e520, 0x43e540), (0x43ba90, 0x43bbe0), (0x43ad50, 0x43afd0), (0x43b330, 0x43b390), (0x432b50, 0x432bd0), (0x432d50, 0x432df0), (0x433280, 0x433300), (0x436750, 0x4367a0), (0x434000, 0x434060), (0x442d10, 0x442d70), (0x443ee0, 0x443f40), (0x444c00, 0x444c50), (0x444040, 0x444090), (0x444700, 0x444740), (0x447790, 0x447830), (0x442cd0, 0x442d10), (0x436820, 0x4368f0), (0x436150, 0x436190), (0x432bd0, 0x432c10), (0x444c50, 0x444cb0), (0x43af70, 0x43afa0), (0x4418d0, 0x4419b0), (0x434650, 0x434740), (0x5da3c0, 0x5da3d0), (0x447990, 0x4479a0), (0x430ac0, 0x430ad0), (0x433220, 0x433240), (0x435710, 0x435730), (0x439920, 0x439ab0), (0x43a060, 0x43a2b0), (0x43d8f0, 0x43dbb0), (0x46f150, 0x46f180), (0x46f350, 0x46f420), (0x46f5c0, 0x46f6f0), (0x444090, 0x444150), (0x445f30, 0x445fb0), (0x436190, 0x436230), (0x445910, 0x445a70), (0x4367a0, 0x436820)], sines=256)
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
    if op in ('Dot', 'Normalize', 'NormXZ'):
        a, b = (r.floats(r.bytes(12), range(0, 12, 4)) for _ in range(2))
        if r.random() < 0.2:  # tiny or huge: the registers' range shows
            put(a, 0, 'fff', *(r.choice([1e-30, 3e-25, 1e25, 2e30, 3e38, 1e-39, 1e-42]) * r.uniform(-1, 1) for _ in range(3)))
        nan_pair(r, a, range(0, 12, 4))
        if op == 'Dot':
            ab = nan_pair(r, a + b, range(0, 24, 4))
            return Call(0x43a970, [OUT, OUT + 12], {OUT: ab}, ret='d')
        return Call(0x43a9a0 if op == 'Normalize' else 0x4387a0, [OUT], {OUT: a})
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
    if op in ('Setup', 'Riders'):
        ncars, segments, flags = r.randrange(0, 4), r.choice([0, 0, r.randrange(1, 9)]), r.choice([0, 1, 1, r.getrandbits(32)])
        riders = r.choice([0, 0, r.randrange(1, 40), r.getrandbits(32)])
        if op == 'Riders':
            segments = 0
        cs, segs, ctypes, stypes = r.bytes(4 * 0x80), r.bytes(0x1000), r.bytes(0x140), r.bytes(0x100)
        put(t, 0x2e8, 'I', ncars)
        put(c, 0x128, 'I', CARS)
        put(c, 0xac, 'f', r.choice([1.0, r.uniform(0.25, 4)]))
        for k in range(3):
            put(stypes, k * 0x40 + 0x24, 'ff', r.uniform(0, 3), r.uniform(0, 3))
        for k in range(4):
            put(ctypes, k * 0x40 + 8, 'IIII', r.choice([0, r.randrange(1, 40)]), *(ORIENT + r.randrange(3) * 0x40 for _ in range(3)))
            put(ctypes, k * 0x40 + 0x1c, 'fff', r.uniform(0, 3), r.uniform(0, 3), r.uniform(0, 3))
        for k in range(4):
            b, ct, old = k * 0x80, r.randrange(4), r.randrange(0, 9)
            put(cs, b, 'I', r.getrandbits(32))
            put(cs, b + 4, 'III', FLAGS + ct * 0x40, r.randrange(4), old)
            put(cs, b + 0x28, 'I', SPL + k * 0x400)
            put(cs, b + 0x74, 'f', r.uniform(-8, 8))
            front, mid, rear = struct.unpack_from('<III', ctypes, ct * 0x40 + 0xc)
            for j in range(9):
                put(segs, k * 0x400 + j * 0x60 + 4, 'I', r.choice([front if j == 0 else mid, mid, ORIENT + r.randrange(3) * 0x40]))
            n = segments or old
            seats = (n - (front != mid) - (rear != mid)) & 0xffffffff
            m = sum(struct.unpack_from('<I', segs, k * 0x400 + j * 0x60 + 4)[0] == mid for j in range(n))
            if m > seats:  # more middle segments than seats divides by zero (sic): give this car its own type
                put(ctypes, 0x100, 'IIII', 0, 1, 2, 3)
                put(cs, b + 4, 'I', FLAGS + 0x100)
        regs = {COAST: c, TYPE: t, CARS: cs, SPL: segs, FLAGS: ctypes, ORIENT: stypes}
        if op == 'Riders':  # through SetupCars(coaster, n, 0, 1) unless n is already +0xf0
            index, most = r.randrange(4), r.choice([r.randrange(-3, 40), 0x7fffffff, r.getrandbits(32)])
            put(t, 0x318, 'I', most & 0xffffffff)
            put(c, 0xf0, 'I', r.choice([r.randrange(0, 40), r.getrandbits(32)]))
            regs[0x790bd0] = put(r.bytes(16), index * 4, 'I', COAST)
            if ncars == 0:
                put(t, 0x2e8, 'I', 1)
            return Call(0x43b330, [index, r.choice([r.randrange(-3, 40), r.getrandbits(32), riders])], regs)
        return Call(0x43ba90, [COAST, riders, segments, flags], regs)
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


def registry(r, op):
    if op == 'Count':
        return Call(0x438720, [], {0x792214: r.bytes(4)}, ret='a')
    if op == 'LinkNodes':
        links, n = r.bytes(0x400), r.randrange(1, 8)
        for k in range(n):
            put(links, k * 0x60 + 0x18, 'I', NODES + (k + 1) * 0x60 if k + 1 < n else 0)
            put(links, k * 0x60 + 0x54, 'I', SPL + k * 0x100)
        spl = r.bytes(0x800)
        for k in range(n):
            put(spl, k * 0x100 + 0x90, 'I', SPL + k * 0x100 + 0x80)
        out = r.choice([0, OUT])
        return Call(0x438730, [r.randrange(n), out], {NODES: links, SPL: spl, OUT: r.bytes(4),
                                                     0x790fe0: put(bytearray(4), 0, 'I', NODES)}, ret='a')
    if op == 'Pair':
        index = r.randrange(4)
        return Call(0x43e0b0, [index, OUT, OUT + 4], {COAST: r.bytes(0x200), OUT: r.bytes(8),
                                                     0x790bd0: put(r.bytes(16), index * 4, 'I', COAST)})
    if op == 'EntryPos':
        index, slot = r.randrange(4), r.randrange(6)
        ride, a, b, entries, objs = r.bytes(0x10), r.bytes(0x30), r.bytes(8), r.bytes(6 * 0x14), r.bytes(0x100)
        put(ride, 8, 'I', NODES)
        put(a, 0x28, 'I', SPL)
        put(b, 4, 'I', FORCE)
        put(entries, slot * 0x14 + 4, 'I', r.choice([0, YAW]))
        put(objs, 0x30, 'III', r.f(), r.f(), r.f())
        return Call(0x4386d0, [index, OUT, OUT + 4], {COAST: ride, NODES: a, SPL: b, FORCE: entries, YAW: objs,
                                                     OUT: put(r.bytes(16), 0, 'I', slot),
                                                     0x7a4610: put(r.bytes(16), index * 4, 'I', COAST)})
    if op == 'SearchReset':
        return Call(0x43ea90, [OUT], {OUT: r.bytes(0x20)})
    # PieceAt: a chain of pieces on through +0x64
    pieces, n = r.bytes(0x800), r.randrange(1, 8)
    for k in range(n):
        put(pieces, k * 0x100 + 0x64, 'I', NODES + (k + 1) % n * 0x100)
    return Call(0x43e520, [COAST, r.randrange(12)], {COAST: put(r.bytes(0x20), 0x18, 'I', NODES), NODES: pieces}, ret='a')


def contacts(r, op):
    """Nodes 0x100 apart: +8 flags, +0x64 next in the owner's list, +0xb0 count, +0xb4 contacts."""
    n = r.randrange(1, 7)
    nodes, owner = r.bytes(0x700), r.bytes(0x200)
    ptrs = [NODES + k * 0x100 for k in range(n)]
    x = r.randrange(n)
    listed = [k for k in range(n) if r.random() < 0.8 or k == x] if r.random() < 0.9 else []
    for i, k in enumerate(listed):
        put(nodes, k * 0x100 + 0x64, 'I', ptrs[listed[i + 1]] if i + 1 < len(listed) else 0)
    put(owner, 0x54, 'I', ptrs[listed[0]] if listed else 0)
    for k in range(n):
        b = k * 0x100
        put(nodes, b + 8, 'I', r.getrandbits(32) & ~0x600000 | r.choice([0, 0x400000, 0x200000, 0x600000]))
        c = r.randrange(0, 9)
        put(nodes, b + 0xb0, 'I', c)
        for j in range(8):
            v = r.choice([ptrs[x], ptrs[x], r.choice(ptrs), 0]) if j < c else r.choice([0, r.getrandbits(32), ptrs[x]])
            if k == x and j < c:
                v = r.choice(ptrs)  # held contacts are followed
            put(nodes, b + 0xb4 + 4 * j, 'I', v)
    if op in ('ClearAll', 'ClearPair'):
        for k in range(n):  # every node's held contacts are followed
            for j in range(struct.unpack_from('<I', nodes, k * 0x100 + 0xb0)[0]):
                put(nodes, k * 0x100 + 0xb4 + 4 * j, 'I', r.choice(ptrs))
        if op == 'ClearPair':  # node, then its partner (+0x68)
            put(nodes, x * 0x100 + 0x68, 'I', r.choice(ptrs + [0]))
            return Call(0x43af70, [ptrs[x], COAST], {COAST: owner, NODES: nodes})
        return Call(0x43afa0, [COAST], {COAST: owner, NODES: nodes})
    return Call(0x43ad50, [COAST, ptrs[x]], {COAST: owner, NODES: nodes})


def train(r, op):
    if op == 'Refresh':  # pieces +8 flags, +0x64 next, +0x6c, +0x90 -> {count, 0x58-byte nodes}
        n = r.randrange(0, 5)
        track, pieces, lists, nodes = r.bytes(0x40), r.bytes(0x500), r.bytes(0x50), r.bytes(5 * 4 * 0x58)
        put(track, 0x18, 'I', NODES if n else 0)
        for k in range(n):
            put(pieces, k * 0x100 + 8, 'I', r.getrandbits(32))
            put(pieces, k * 0x100 + 0x64, 'If', NODES + (k + 1) * 0x100 if k + 1 < n else 0, 0.0)
            put(pieces, k * 0x100 + 0x6c, 'I', r.f())
            put(pieces, k * 0x100 + 0x90, 'I', SPL + k * 0x10)
            put(lists, k * 0x10, 'II', r.randrange(0, 4), FORCE + k * 4 * 0x58)
        r.floats(nodes, range(0, len(nodes), 4))
        return Call(0x432b50, [], {COAST: track, NODES: pieces, SPL: lists, FORCE: nodes,
                                   0x790fe4: put(bytearray(4), 0, 'I', COAST)})
    if op == 'TrainLen':  # car +4 count, +0x100 train (+0x18 type: bc, c0, c4; +0x34 cars), cars +0xe8
        n = r.randrange(0, 7)
        car, train, types, cars = r.bytes(0x200), r.bytes(0x60), r.bytes(0x100), r.bytes(6 * 0x100)
        put(car, 4, 'I', n)
        put(car, 0x100, 'I', SPL)
        put(train, 0x18, 'I', FORCE)
        for k in range(n):
            put(train, 0x34 + 4 * k, 'I', r.choice([0, COAST, NODES + r.randrange(6) * 0x100, NODES + k * 0x100]))
        r.floats(types, (0xbc, 0xc0, 0xc4))
        r.floats(car, (0xe8,))
        r.floats(cars, range(0xe8, len(cars), 0x100))
        nan_pair(r, types, (0xbc, 0xc0, 0xc4))
        if r.random() < 0.2:
            nan_pair(r, cars, range(0xe8, len(cars), 0x100), 1)
        return Call(0x433280, [r.choice([COAST] * 9 + [0])], {COAST: car, SPL: train, FORCE: types, NODES: cars}, ret='d')
    n = r.randrange(0, 6)
    tr, cars, parts, types = r.bytes(0x60), r.bytes(6 * 0x180), r.bytes(0xc0), r.bytes(0x100)
    put(tr, 0x30, 'I', n)
    for k in range(n):
        put(tr, 0x34 + 4 * k, 'I', NODES + k * 0x180)
    for k in range(6):
        put(cars, k * 0x180 + 0x100, 'I', SPL + r.randrange(6) * 0x20)
    for k in range(6):
        put(parts, k * 0x20 + 0x18, 'I', FORCE + r.randrange(4) * 0x40)
    if op == 'Pick':
        return Call(0x436750, [NODES + r.randrange(6) * 0x180], {NODES: cars, SPL: parts, FORCE: types})
    return Call(0x432d50, [r.getrandbits(32), COAST], {COAST: tr, NODES: cars, SPL: parts, FORCE: types,
                                    0x790afc: put(bytearray(4), 0, 'I', r.choice([0, 0, 0, 1]))})


def tiles(r, op):
    """Links (0x790fe0, next +0x18) with chains (+0x4c, next +0x28) of keyed (+8) entries."""
    width = r.choice([r.randrange(1, 9), r.getrandbits(32)])
    world, links, ents = r.bytes(0x80), r.bytes(4 * 0x60), r.bytes(16 * 0x30)
    put(world, 0x6c, 'I', TYPE + 0x40)
    put(world, 0x40 + 0x18, 'I', width)
    nl = r.randrange(0, 5)
    free = list(range(16))
    r.shuffle(free)
    for k in range(nl):
        put(links, k * 0x60 + 0x18, 'I', CARS + (k + 1) * 0x60 if k + 1 < nl else 0)
        chain = [free.pop() for _ in range(r.randrange(0, 4))]
        put(links, k * 0x60 + 0x4c, 'I', NODES + chain[0] * 0x30 if chain else 0)
        for j, e in enumerate(chain):
            put(ents, e * 0x30 + 0x28, 'I', NODES + chain[j + 1] * 0x30 if j + 1 < len(chain) else 0)
            put(ents, e * 0x30 + 4, 'B', r.choice([0, 0, 1, 4, 5, 2, 8, r.getrandbits(8)]))
            put(ents, e * 0x30 + 8, 'I', r.choice([r.randrange(0, 12), r.getrandbits(32)]))
    x, y = r.choice([(r.randrange(0, 6), r.randrange(0, 3)), (r.getrandbits(32), r.getrandbits(32))])
    if op == 'Key':
        return Call(0x436150, [x + y], {TYPE: world, CARS: links, NODES: ents,
                                        0x790fe0: put(bytearray(4), 0, 'I', CARS if nl else 0)}, ret='a')
    return Call(0x434000, [x, y, r.choice([OUT, 0])],
                {TYPE: world, CARS: links, NODES: ents, OUT: r.bytes(4),
                 0x7a0854: put(bytearray(4), 0, 'I', TYPE),
                 0x790fe0: put(bytearray(4), 0, 'I', CARS if nl else 0)}, ret='a')


def current(r, op):
    """The current link (0x790fec)."""
    link = r.bytes(0x80)
    if op in ('BeginEnd', 'SetEnd', 'ClearEnd'):  # with type flag 2: 0x79c664 and 0x79c658 flag 0x40
        put(link, 4, 'I', TYPE)
        return Call({'BeginEnd': 0x442cd0, 'SetEnd': 0x444c50, 'ClearEnd': 0x444c80}[op], [], {COAST: link, TYPE: r.bytes(4), 0x79c658: r.bytes(4), 0x79c664: r.bytes(4),
                                   0x790fec: put(bytearray(4), 0, 'I', r.choice([COAST] * 9 + [0]))}, ret='a')
    if op == 'Tools':  # the end check, type flags 0x80 / 0x100, +0x3c flags 1 / 2
        put(link, 4, 'I', TYPE)
        put(link, 0x58, 'I', NODES)
        put(link, 0x60, 'I', NODES + 0x40)
        return Call(0x445910, [r.choice([OUT, 0])], {COAST: link, TYPE: r.bytes(4), NODES: r.bytes(0x80), OUT: r.bytes(4),
                                                    0x79c638: r.bytes(4), 0x79c6e0: r.bytes(4),
                                                    0x790fec: put(bytearray(4), 0, 'I', COAST)}, ret='a')
    if op in ('End', 'Marked'):  # +4 type flag 2; +0x3c flags 1, 2; end pieces +0x58/+0x60 flag 0x80 (+8)
        put(link, 4, 'I', TYPE)
        put(link, 0x58, 'I', NODES)
        put(link, 0x60, 'I', NODES + 0x40)
        return Call(0x442d10 if op == 'End' else 0x444c00, [], {COAST: link, TYPE: r.bytes(4), NODES: r.bytes(0x80),
                                   0x79c658: r.bytes(4), 0x79c664: r.bytes(4),
                                   0x790fec: put(bytearray(4), 0, 'I', r.choice([COAST] * 9 + [0]))},
                    ret='v' if op == 'End' else 'a')
    # trains on +0x4c (next +0x28, +4 flag 1, cars +0x30/+0x34), cars' ride index +0x70
    rides = [r.choice([1, 2, 3, r.getrandbits(32)]) for _ in range(8)]
    trains, cars = r.bytes(4 * 0x60), r.bytes(12 * 0x80)
    nt = r.randrange(0, 5)
    put(link, 0x54, 'I', r.choice([0, 1, r.getrandbits(32)]))
    put(link, 0x4c, 'I', CARS if nt else 0)
    for k in range(nt):
        put(trains, k * 0x60 + 0x28, 'I', CARS + (k + 1) * 0x60 if k + 1 < nt else 0)
        put(trains, k * 0x60 + 4, 'B', r.choice([0, 1, 2, r.getrandbits(8)]))
        nc = r.randrange(0, 4)
        put(trains, k * 0x60 + 0x30, 'I', nc)
        for j in range(nc):
            put(trains, k * 0x60 + 0x34 + 4 * j, 'I', NODES + r.randrange(12) * 0x80)
    for k in range(12):
        put(cars, k * 0x80 + 0x70, 'I', r.randrange(8))
    return Call(0x443ee0, [r.choice(rides[:4] + [r.getrandbits(32)])],
                {COAST: link, CARS: trains, NODES: cars, 0x7a4610: struct.pack('<8I', *rides),
                 0x790fec: put(bytearray(4), 0, 'I', COAST)}, ret='a')


def pieces(r, op):
    """A chain of pieces (+0 index, +8 flags, +0x64 next, +0x68 previous) in NODES."""
    n = r.randrange(1, 6)
    ps = r.bytes(n * 0x80 + 0x80)
    base = r.choice([0, r.randrange(0, 1 << 31), 0xfffffffe - n, 0x80000000 - r.randrange(1, n + 1)])
    for k in range(n):
        put(ps, k * 0x80 + 0x64, 'I', NODES + (k + 1) * 0x80 if k + 1 < n else 0)
        put(ps, k * 0x80, 'I', base + k)
    if op == 'Renumber':  # the previous piece is outside the chain
        put(ps, n * 0x80, 'I', r.choice([r.randrange(0, 9), r.getrandbits(32), 0xffffffff]))
        put(ps, 0x68, 'I', r.choice([NODES + n * 0x80, 0]))
        return Call(0x444700, [r.choice([NODES] * 9 + [0])], {NODES: ps})
    if op == 'Append':  # a new piece (slot n) after the track's last (+0x24)
        track = r.bytes(0x40)
        k = r.randrange(n)
        put(track, 0x24, 'I', NODES + k * 0x80)
        put(track, 0x1c, 'I', r.choice([NODES + k * 0x80, NODES + r.randrange(n) * 0x80, 0]))
        return Call(0x436190, [NODES + n * 0x80, COAST, r.choice([0, 1, r.getrandbits(32)])], {NODES: ps, COAST: track})
    if op == 'Remove':  # piece k as (or not as) the track's last; its previous is set when flagged
        track = r.bytes(0x40)
        k = r.randrange(n)
        put(ps, 0x68, 'I', r.choice([0, NODES + n * 0x80]))  # outside the chain, or none
        put(ps, n * 0x80 + 0x68, 'I', r.choice([0, NODES + n * 0x80]))  # which has none, or itself
        for j in range(1, n):
            put(ps, j * 0x80 + 0x68, 'I', NODES + (j - 1) * 0x80)
        last = r.choice([NODES + k * 0x80, NODES + r.randrange(n) * 0x80])
        put(track, 0x24, 'I', last)
        put(track, 0x1c, 'I', r.choice([last, NODES + r.randrange(n) * 0x80, 0]))
        if k == 0 and not any(ps[0x68:0x6c]) and track[0x1c:0x20] != track[0x24:0x28]:  # the previous would be flagged
            put(ps, k * 0x80 + 8, 'I', r.getrandbits(32) & ~0x8000)
        return Call(0x4367a0, [NODES + k * 0x80, COAST], {NODES: ps, COAST: track})
    if op == 'ClearSel':
        i, j = sorted((r.randrange(n), r.randrange(n)))
        return Call(0x445f30, [], {NODES: ps, COAST: r.bytes(4), 0x7925d0: r.bytes(4),
                                   0x79c628: put(bytearray(4), 0, 'I', r.choice([NODES + i * 0x80] * 4 + [0])),
                                   0x79c62c: put(bytearray(4), 0, 'I', NODES + j * 0x80),
                                   0x790fec: put(bytearray(4), 0, 'I', COAST)})
    if op == 'SetFirst':  # a (0x79c628) and the new piece each against b (0x79c62c)
        i, j, k = r.randrange(n), r.randrange(n), r.randrange(n)
        if r.random() < 0.3:  # ties go to b, which must come first
            i, j = k, 0
            put(ps, 0, 'I', base + k)
        return Call(0x444090, [r.choice([NODES + k * 0x80] * 4 + [0]), r.choice([0, 1, r.getrandbits(32)])],
                    {NODES: ps, COAST: r.bytes(4),
                     0x79c628: put(bytearray(4), 0, 'I', r.choice([NODES + i * 0x80] * 4 + [0])),
                     0x79c62c: put(bytearray(4), 0, 'I', NODES + j * 0x80),
                     0x790fec: put(bytearray(4), 0, 'I', COAST)})
    i, j = r.randrange(n), r.randrange(n)
    if r.random() < 0.3:  # equal indices: from 0x79c62c, which must come first
        j, i = sorted((i, j))
        i = j if r.random() < 0.3 else i
        put(ps, j * 0x80, 'I', base + i)
    return Call(0x444040, [], {NODES: ps, 0x79c628: put(bytearray(4), 0, 'I', r.choice([NODES + i * 0x80] * 9 + [0])),
                               0x79c62c: put(bytearray(4), 0, 'I', NODES + j * 0x80)}, ret='a')


def ride_train(r, op):
    """A train (+4 byte flag 0x20, cars +0x30 / +0x34) of cars (+8 flag 0x2000, +0x70)."""
    train, cars = r.bytes(0x60), r.bytes(8 * 0x80)
    put(train, 0x30, 'I', r.choice([r.randrange(0, 7), r.getrandbits(32)]) if op == 'RideTrain' else r.randrange(0, 7))
    for k in range(8):
        put(train, 0x34 + 4 * k, 'I', NODES + k * 0x80)
    if op == 'CarValue':
        return Call(0x4477d0, [CARS, r.choice([r.randrange(0, 8), r.randrange(0, 3)])], {CARS: train, NODES: cars}, ret='a')
    rides, owner = r.bytes(2 * 0x100), r.bytes(0x110)
    put(rides, 0xd4, 'I', SPL)
    put(rides, 0x100 + 0xd4, 'I', SPL)
    put(owner, 0x100, 'I', CARS)
    return Call(0x447790, [r.randrange(2), OUT], {CARS: train, TYPE: rides, SPL: owner, OUT: r.bytes(4),
                                                0x7a4610: struct.pack('<2I', TYPE, TYPE + 0x100)}, ret='a')


def by_id(r, op):
    """Links at 0x790fe0 (next +0x18, id +0x10, flags +0, +0x124 -> +0x34 -> +0xc)."""
    if op == 'Bit18':
        table = r.choice([0x7ae6a8, 0x7a4610])
        return Call(0x436820 if table == 0x7ae6a8 else 0x436840, [r.randrange(2)],
                    {TYPE: r.bytes(8), table: struct.pack('<2I', TYPE, TYPE + 4)}, ret='a')
    n = r.randrange(0, 5)
    links, deep = r.bytes(4 * 0x150), r.bytes(0x40)
    ids = [r.choice([1, 2, 3, r.getrandbits(32)]) for _ in range(n)]
    for k in range(n):
        put(links, k * 0x150 + 0x18, 'I', CARS + (k + 1) * 0x150 if k + 1 < n else 0)
        put(links, k * 0x150 + 0x10, 'I', ids[k])
        put(links, k * 0x150 + 0x124, 'I', SPL)
    put(deep, 0x34, 'I', SPL + 0x20)
    want = r.choice(ids + [r.choice([1, 2, 3]), r.getrandbits(32)] + [i ^ 0x10000 for i in ids])
    regs = {CARS: links, SPL: deep, 0x790fe0: put(bytearray(4), 0, 'I', CARS if n else 0)}
    if op in ('ObjLink', 'ObjOpen', 'ObjBare', 'ObjOther', 'ObjParams'):  # object +0x20 is the id
        if op == 'ObjParams':
            want = r.choice(ids) if n else want
        for k in range(n):
            put(links, k * 0x150 + 0x3c, 'I', r.getrandbits(32))
            put(links, k * 0x150 + 0x140, 'I', r.choice([0, 0, r.getrandbits(32)]))
        regs[TYPE] = put(r.bytes(0x40), 0x20, 'I', want)
        if op == 'ObjParams':
            if not n:
                return Call(0x43e120 if r.random() < 0.5 else 0x43e140, [],
                            {0x792404: r.bytes(4), 0x7925e0: r.bytes(4)})
            regs[OUT] = r.bytes(16)
            return Call(0x43e0e0, [TYPE, OUT, OUT + 4, OUT + 8, OUT + 12], regs)
        if op == 'ObjOther':
            regs[0x790fec] = put(bytearray(4), 0, 'I', CARS + r.randrange(max(n, 1)) * 0x150)
        return Call({'ObjLink': 0x4418f0, 'ObjOpen': 0x441910, 'ObjBare': 0x441940, 'ObjOther': 0x441970}[op],
                    [r.choice([TYPE] * 9 + [0])], regs, ret='a')
    if op == 'ById':
        return Call(0x436860, [want], regs, ret='a')
    if op == 'IdValue':
        return Call(0x436880, [want], regs, ret='a')
    return Call(0x4368c0, [want, r.getrandbits(32)], regs)


def event(r, op):
    return Call(0x432bd0, [r.getrandbits(32) for _ in range(4)], {0x791050: r.bytes(8), 0x791ce8: r.bytes(16)}, ret='a')


def small(r, op):
    if op == 'Flag1':
        return Call(0x447990, [], {0x79c658: r.bytes(4)}, ret='a')
    if op == 'SetCur':
        return Call(0x430ac0, [r.getrandbits(32)], {0x790fec: r.bytes(4)})
    if op == 'Moved':
        return Call(0x433220, [OUT, r.choice([0, 1, r.getrandbits(32)])], {OUT: r.bytes(4)})
    if op == 'Position':  # out may overlap the source
        return Call(0x435710, [OUT, r.choice([OUT + 0x40, OUT + 0x34, OUT + 0x2c, OUT + 0x30])], {OUT: r.bytes(0x50)})
    if op == 'SelGet':
        return Call(0x4418d0, [], {0x79c640: r.bytes(4)}, ret='a')
    if op == 'SelSet':
        return Call(0x4418e0, [r.getrandbits(32)], {0x79c640: r.bytes(4)})
    if op == 'State':
        outs = [OUT, OUT + 4, OUT + 8, OUT + 12] if r.random() < 0.7 else [OUT + 4 * r.randrange(2) for _ in range(4)]
        return Call(0x434710, outs, {OUT: r.bytes(16), 0x79221c: r.bytes(4), 0x790fd8: r.bytes(4),
                                     0x79104c: r.bytes(4), 0x791cf8: r.bytes(4)})
    if op == 'Changed':
        return Call(0x4346c0, [OUT, r.choice([0, 1, r.getrandbits(32)])], {OUT: r.bytes(4)})
    if op == 'LostPair':
        nodes = r.bytes(0x100)
        put(nodes, 0x68, 'I', r.choice([0, NODES, NODES + 0x80]))
        return Call(0x4346f0, [NODES], {NODES: nodes})
    # car type: shortest +0xbc, longest +0xc0
    t = r.bytes(0x100)
    r.floats(t, (0xbc, 0xc0))
    x = r.f()
    if r.random() < 0.15:
        put(t, 0xc0, 'I', struct.unpack_from('<I', t, 0xbc)[0])
    if r.random() < 0.25:
        x = r.choice([0x7fc00000, 0xffc00000])
        put(t, r.choice([0xbc, 0xc0]), 'I', r.choice([0x7fc00000, 0xffc00000]))
    return Call({'Unlerp': 0x434650, 'Scale': 0x434680, 'Lerp': 0x4346a0}[op], [x, TYPE], {TYPE: t}, ret='d')


def bounds(r, op):
    """Owner +0x54 nodes (+0x64 next, +0xfc point index, +0x100 -> +0x18 type, +0x58 pad),
    +0x9c points 0x38 apart with a position at +4."""
    n = r.randrange(1, 6)
    owner, nodes, parts, types, points = r.bytes(0x200), r.bytes(6 * 0x110), r.bytes(0x100), r.bytes(0x100), r.bytes(30 * 0x38)
    put(owner, 0x54, 'I', NODES)
    put(owner, 0x9c, 'I', FORCE)
    index = r.randrange(0, 4)
    for k in range(n):
        b = k * 0x110
        put(nodes, b + 0x64, 'I', NODES + (k + 1) * 0x110 if k + 1 < n else 0)
        put(nodes, b + 0xfc, 'I', index)
        index = max(0, min(29, index + r.choice([-1, 0, 1, 2, 3, 5])))
        put(nodes, b + 0x100, 'I', SPL + r.randrange(4) * 0x20)
    for k in range(4):
        put(parts, k * 0x20 + 0x18, 'I', r.choice([0, TYPE, TYPE + 0x40]))
    r.floats(types, (0x58, 0x98))
    if r.random() < 0.3:
        exact = [r.choice([0.0, -0.0, 1.0, 2.0, -1.0, 0.5]) for _ in range(4)]
        for o in range(4, len(points), 4):
            put(points, o, 'f', r.choice(exact))
    else:
        r.floats(points, range(0, len(points), 4))
    nan_pair(r, points, range(4, 30 * 0x38, 0x38), 0.2)
    if r.random() < 0.2:  # NaN pads against NaN boxes, for the operand order
        for o in (0x58, 0x98):
            put(types, o, 'I', r.choice([0x7fc00000, 0xffc00000]))
        for o in range(4, len(points), 0x38):
            put(points, o, 'I', r.choice([0x7fc00000, 0xffc00000]))
    return Call(0x439920, [COAST], {COAST: owner, NODES: nodes, SPL: parts, TYPE: types, FORCE: points})


def segments(r, op):
    """Contacts_SegmentsInBox(owner, node, next, box, out, count): points between the
    nodes' indices (+0xfc), tested against a padded box."""
    owner, nodes, parts, types = r.bytes(0x200), r.bytes(2 * 0x110), r.bytes(0x100), r.bytes(0x100)
    points, box = r.bytes(30 * 0x38), r.bytes(0x18)
    put(owner, 0x9c, 'I', FORCE)
    s0 = r.randrange(0, 12)
    put(nodes, 0xfc, 'I', s0)
    put(nodes, 0x110 + 0xfc, 'I', s0 + r.randrange(0, 17))
    for k in range(2):
        put(nodes, k * 0x110 + 0x100, 'I', SPL + r.randrange(4) * 0x20)
    for k in range(4):
        put(parts, k * 0x20 + 0x18, 'I', r.choice([0, TYPE, TYPE + 0x40]))
    mode = r.random()
    if mode < 0.6:  # small exact values, so ends fall in, out and on the box
        grid = [x * 0.5 for x in range(-6, 7)] + [-0.0]
        for o in range(0, len(points), 4):
            put(points, o, 'f', r.choice(grid))
        lo = [r.choice(grid) for _ in range(3)]
        for j in range(3):
            put(box, 4 * j, 'f', lo[j])
            put(box, 12 + 4 * j, 'f', lo[j] + r.choice([0, 0.5, 1, 2, 3, -1]))
        for o in (0x58, 0x98):
            put(types, o, 'f', r.choice([0, 0.5, 1, -0.5, -0.0]))
    else:
        r.floats(points, range(0, len(points), 4))
        r.floats(box, range(0, 0x18, 4))
        r.floats(types, (0x58, 0x98))
    nan_pair(r, points, range(4, 30 * 0x38, 4), 0.2)
    if r.random() < 0.15:  # NaN pads and ends, for the operand order
        for o in (0x58, 0x98):
            put(types, o, 'I', r.choice([0x7fc00000, 0xffc00000]))
        for o in range(4, len(points), 4):
            if r.random() < 0.5:
                put(points, o, 'I', r.choice([0x7fc00000, 0xffc00000]))
    if r.random() < 0.1:
        put(box, r.choice(range(0, 0x18, 4)), 'I', r.choice([0x7fc00000, 0xffc00000]))
    return Call(0x43a060, [COAST, NODES, NODES + 0x110, OUT, ORIENT, FLAGS],
                {COAST: owner, NODES: nodes, SPL: parts, TYPE: types, FORCE: points, OUT: box,
                 ORIENT: r.bytes(17 * 0x1c), FLAGS: r.bytes(4)})


def ride_orient(r, op):
    """RideObject_Orient(object, angles, v): object +0xc ride index; ride +8 -> +4 -> +0x78
    -> matrix at +0x10."""
    obj, rides, holder, inner, mat = r.bytes(0x20), r.bytes(0x20), r.bytes(0x10), r.bytes(0x80), r.bytes(0x60)
    idx = r.randrange(2)
    put(obj, 0xc, 'I', idx)
    put(rides, idx * 0x10 + 8, 'I', SPL)
    put(holder, 4, 'I', FORCE)
    put(inner, 0x78, 'I', CARS)
    angles, v = r.bytes(0x10), r.bytes(12)
    mode = r.random()
    if mode < 0.5:  # whole steps of the table, and halves (fistp ties)
        for o in (4, 8, 12):
            put(angles, o, 'f', r.randrange(-300, 300) * (math.pi / 128) * r.choice([1, 1, 0.5, 1.5]))
    else:
        r.floats(angles, (4, 8, 12))
    r.floats(v, (0, 4, 8))
    nan_pair(r, v, (0, 4, 8), 0.15)
    if r.random() < 0.15:  # a lone NaN, for which operand of a subtraction it is
        put(v, r.choice((0, 4, 8)), 'I', r.choice([0x7fc00000, 0xffc00000]))
    return Call(0x43d8f0, [COAST, OUT, r.choice([ORIENT] * 5 + [0])],
                {COAST: obj, TYPE: rides, 0x7a4610: struct.pack('<2I', TYPE, TYPE + 0x10), SPL: holder, FORCE: inner, CARS: mat, OUT: angles, ORIENT: v})


KINDS = {'Reset': ratings, 'Step': ratings, 'Finish': ratings, 'Point': spline, 'Tangent': spline, 'Set': orient,
         'Init': orient, 'Frame': frame, 'At': frame,
         'RoundUp': helpers, 'Lerp': helpers, 'Dot': helpers, 'Normalize': helpers, 'NormXZ': helpers, 'Swing': cars, 'Space': cars, 'Segs': cars, 'Setup': cars, 'Riders': cars, 'Extent': cars, 'Drop': cars, 'Window': cars, 'InWindow': cars, 'Dist': cars,
         'NodeLerp': nodes, 'PastEnd': nodes, 'Zone': nodes, 'Merge': keys, 'Append': lists,
         'Link': queue, 'QPush': queue, 'QAdd': queue, 'QRoom': queue, 'QPop': queue, 'QAdvance': queue,
         'CNext': cursor, 'CPrev': cursor, 'CPoint': cursor,
         'Count': registry, 'LinkNodes': registry, 'Pair': registry, 'EntryPos': registry, 'SearchReset': registry,
         'PieceAt': registry, 'Contacts': contacts, 'ClearAll': contacts, 'Refresh': train, 'Couple': train, 'TrainLen': train, 'Pick': train, 'Tile': tiles, 'End': current, 'OnRide': current, 'Marked': current, 'Select': pieces, 'Renumber': pieces, 'RideTrain': ride_train, 'CarValue': ride_train, 'BeginEnd': current, 'Bit18': by_id, 'ById': by_id, 'IdValue': by_id, 'SetById': by_id, 'Key': tiles, 'Event': event, 'SetEnd': current, 'ClearEnd': current, 'ClearPair': contacts, 'ObjLink': by_id, 'ObjOpen': by_id, 'ObjBare': by_id, 'SelGet': small, 'SelSet': small, 'State': small, 'Changed': small, 'LostPair': small, 'Unlerp': small, 'Scale': small, 'Lerp': small, 'Flag1': small, 'SetCur': small, 'Moved': small, 'Position': small, 'Bounds': bounds, 'Segments': segments, 'RideOrient': ride_orient, 'SetFirst': pieces, 'Append': pieces, 'ClearSel': pieces, 'Tools': current, 'Remove': pieces, 'ObjOther': by_id, 'ObjParams': by_id}
OPS = [k for k in os.environ.get('OPS', ','.join(KINDS)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    return KINDS[op](r, op)


if __name__ == '__main__':
    B.main(case)
