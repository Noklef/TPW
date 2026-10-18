# The float pair made unit length (src/game/math/vec2: 0x5787b0) against the original: any floats, zero pairs, NaNs and infinities;
# and the integer pair's length (0x549020): any ints, small ones, ones about the 0x8000 / 0x7fff limits that halve them and INT_MIN.
# The integer area's point test (0x5493f0): see contains().
# usage: vec2.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x5787b0, 0x578800), (0x549020, 0x54906e), (0x67a830, 0x67a860), (0x5493f0, 0x5494c6)])
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


def case(r):
    op = r.deal([0x5787b0, 0x549020, 0x5493f0])
    if op == 0x549020:
        return length(r)
    if op == 0x5493f0:
        return contains(r)
    pair = [r.f(), r.f()]
    for i in range(2):
        if r.random() < 0.15:
            pair[i] = r.choice(EDGE)
    return Call(0x5787b0, [RAM], {RAM: struct.pack('<2I', *pair)}, conv='t')


if __name__ == '__main__':
    B.main(case)
