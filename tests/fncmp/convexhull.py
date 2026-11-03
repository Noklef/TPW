# A hull grown a point (src/game/math/convexhull: 0x56bd60) against the original, its memmove as it is. Up to 8 points, around a circle (either way round) or anywhere; the point in or out, on a corner, on a line through
# two (whole numbers), with a NaN rarely. Over 8 points the rebuild can run over the bytes and the count: not benched.
# usage: convexhull.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x56bd60, 0x56c5a0)])
HULL = RAM
NANS = [0x7fc12345, 0x7f812345, 0xffc00000]


def num(r, v, nan):
    return r.choice(NANS) if r.random() < nan else fb(v)


def case(r):
    count = r.deal([0, 1, 1, 2, 2, 2, 2, 3, 3, 4, 5, 6, 8])
    nan = r.deal([0] * 9 + [0.1])
    whole = r.random() < 0.3
    hull = bytearray(r.bytes(0xe0))
    shape = r.deal(['round', 'back', 'any'])
    points = []
    for k in range(count):
        if shape == 'any':
            points.append((r.randrange(-8, 9), r.randrange(-8, 9)) if whole else (r.uniform(-9, 9), r.uniform(-9, 9)))
        else:
            turn = (k + r.random() * 0.8) / max(count, 1) * 2 * math.pi * (1 if shape == 'round' else -1)
            points.append((round(math.cos(turn) * 8), round(math.sin(turn) * 8)) if whole else (math.cos(turn) * 8, math.sin(turn) * 8))
    how = r.deal(['in', 'out', 'far', 'corner', 'line', 'line', 'any'])
    if how == 'in':
        x, z = r.uniform(-1, 1), r.uniform(-1, 1)
    elif how == 'out':
        x, z = r.uniform(-12, 12), r.uniform(-12, 12)
    elif how == 'far':
        turn = r.uniform(0, 7)
        x, z = math.cos(turn) * 50, math.sin(turn) * 50
    elif how == 'corner' and points:
        x, z = r.choice(points)
    elif how == 'line' and len(points) >= 2:
        a, b = r.sample(points, 2) if count > 2 else points
        # On the line: a straight one, level or upright, when whole.
        if count == 2 and r.random() < 0.7:
            axis = r.randrange(0, 3)
            if axis == 0:
                b = (a[0], b[1])
            elif axis == 1:
                b = (b[0], a[1])
            elif r.random() < 0.3:
                b = a
            points[1] = b
        t = r.deal([-1.0, -0.5, 0.0, 0.5, 1.0, 1.5, 2.0])
        x, z = a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t
    else:
        x, z = r.uniform(-9, 9), r.uniform(-9, 9)
    for k, (px, pz) in enumerate(points):
        put(hull, k * 0xc, 'III', num(r, px, nan), r.getrandbits(32), num(r, pz, nan))
    put(hull, 0xd0, 'iI', count, r.getrandbits(32))
    return Call(0x56bd60, [HULL, num(r, x, nan), r.getrandbits(32), num(r, z, nan)], {HULL: hull}, conv='t')


if __name__ == '__main__':
    B.main(case)
