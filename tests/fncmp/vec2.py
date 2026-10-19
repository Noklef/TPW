# The float pair made unit length (src/game/math/vec2: 0x5787b0) against the original: any floats, zero pairs, NaNs and infinities;
# and the integer pair's length (0x549020): any ints, small ones, ones about the 0x8000 / 0x7fff limits that halve them and INT_MIN.
# The integer area's point test (0x5493f0): see contains(). Two areas meeting (0x5491e0): see overlaps().
# usage: vec2.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5787b0, 0x578800), (0x549020, 0x54906e), (0x67a830, 0x67a860), (0x5493f0, 0x5494c6), (0x5491e0, 0x5493ef)])
EDGE = [0, 0x80000000, 0x7fc00000, 0x7f800001, 0x7f800000, 0xff800000, 1, 0x00800000]


def length(r):
    def one():
        k = r.choice([0, 1, 2, 3, 4])
        v = [r.getrandbits(32), r.randrange(-300, 300), r.choice([0x7fff, 0x8000, 0x8001, 0x10000, 0xffff, 0x80000000, 0x7fffffff]), r.randrange(-0x20000, 0x20000), 0][k]
        return (v if r.random() < 0.5 else -v) & 0xffffffff
    return Call(0x549020, [one(), one()], {}, ret='a')


def contains(r):
    """an area {flags, centre x, y, radius / half width, half height}: flags 0x20 (a box) or not, the side bits 2, 4, 8, 0x10 any;
    points on the centre, on its axes, near and past the radius (the circle's root rounding at 24 bits), far (the halving) and INT_MIN apart."""
    cx, cy = r.randrange(-0x10000, 0x10000), r.randrange(-0x10000, 0x10000)
    rx, ry = r.choice([0, 1, 10, 100, 0x8000, 46340, 46341, r.getrandbits(16), r.getrandbits(32) - (1 << 31)]), r.randrange(-5, 0x2000)
    flags = r.getrandbits(32) if r.random() < 0.2 else r.choice([0, 0x20]) | sum(b for b in (2, 4, 8, 0x10) if r.random() < 0.8)

    def off(k):
        k = abs(k)
        return r.choice([0, 0, r.randrange(-k - 2, k + 3), k, -k, k + 1, -k - 1, r.getrandbits(32) - (1 << 31), 0x80000000, r.randrange(-0x30000, 0x30000)])
    k = r.choice([rx, abs(rx) * 7 // 10, ry])
    x, y = (cx - off(k)) & 0xffffffff, (cy - off(k)) & 0xffffffff
    area = struct.pack('<Iiiii', flags, cx, cy, rx, ry)
    return Call(0x5493f0, [x, y, 0 if r.random() < 0.02 else RAM], {RAM: area}, ret='a')

def overlaps(r):
    """two areas (or the same one twice): side flags all four (0x1e), some or none, now and then any; centres the same, on a line, 0x1800 apart
    in |dx| + |dy| (split any way, either sign), near and far (the halving) and INT_MIN apart; radii either larger, negative, wrapping when summed."""
    def flags():
        k = r.random()
        if k < 0.35:
            return 0x1e | r.choice([0, 0x20, 1, 0x40])
        if k < 0.85:
            return sum(b for b in (2, 4, 8, 0x10) if r.random() < 0.6) | r.choice([0, 0x20])
        return r.getrandbits(32)

    def radius():
        return r.choice([0, 1, 10, 100, 0x800, 0x1000, 0x8000, 46340, r.getrandbits(16), r.getrandbits(31), 0x7fffffff, -5, r.getrandbits(32) - (1 << 31)])
    ax, ay = r.randrange(-0x10000, 0x10000), r.randrange(-0x10000, 0x10000)
    k = r.random()
    if k < 0.15:
        dx, dy = 0, 0
    elif k < 0.35:
        dx = r.randrange(0x1801)
        dx, dy = dx * r.choice([1, -1]), (0x1800 - dx) * r.choice([1, -1])
        if r.random() < 0.3:
            dx += r.choice([1, -1])
    elif k < 0.45:
        dx, dy = r.choice([(0, r.randrange(-0x3000, 0x3000)), (r.randrange(-0x3000, 0x3000), 0)])
    elif k < 0.55:
        dx, dy = r.choice([0x80000000, r.getrandbits(32)]), r.choice([0x80000000, r.getrandbits(32), 0])
    else:
        dx, dy = r.randrange(-0x30000, 0x30000), r.randrange(-0x30000, 0x30000)
    ra, rb = [radius(), radius()], [radius(), radius()]
    if r.random() < 0.25:   # touching: the larger radii summing to the distance (a 3-4-5 multiple or on an axis), or one off
        m = r.randrange(1, 0x800)
        dx, dy = r.choice([(3 * m, 4 * m), (-4 * m, 3 * m), (5 * m, 0), (0, -5 * m)])
        big = r.randrange(5 * m + 1) + r.choice([0, 0, 1, -1])
        ra, rb = [big, r.randrange(-5, big + 1)], [5 * m - big, r.randrange(-5, max(5 * m - big, -4) + 1)]
        r.shuffle(ra)
        r.shuffle(rb)
    a = struct.pack('<Iiiii', flags(), ax, ay, *ra) + r.bytes(0xc)
    b = struct.pack('<IIIii', flags(), (ax + dx) & 0xffffffff, (ay + dy) & 0xffffffff, *rb) + r.bytes(0xc)
    first, second = (RAM, RAM + 0x20) if r.random() < 0.5 else (RAM + 0x20, RAM)
    if r.random() < 0.04:
        second = first
    return Call(0x5491e0, [first, second], {RAM: a + b}, ret='a')


def case(r):
    op = r.deal([0x5787b0, 0x549020, 0x5493f0, 0x5491e0])
    if op == 0x549020:
        return length(r)
    if op == 0x5493f0:
        return contains(r)
    if op == 0x5491e0:
        return overlaps(r)
    pair = [r.f(), r.f()]
    for i in range(2):
        if r.random() < 0.15:
            pair[i] = r.choice(EDGE)
    return Call(0x5787b0, [RAM], {RAM: struct.pack('<2I', *pair)}, conv='t')


if __name__ == '__main__':
    B.main(case)
