# Two cars' shapes against each other (src/game/coaster/carprisms: 0x43a2b0, 0x43aa10) against the original, with Vec3_Dot, Vec3_Normalize and the CRT's acos. Touch: two shapes of 0-6 points (the same or a smaller
# count for the second's edges), cars close enough to meet now and then. InQuad: a flat quad and a point in it or out of it. Now and then a NaN.
# usage: carprisms.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Touch,Quad)
import os
import math
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb, put  # noqa: E402

B = Bench([(0x43a2b0, 0x43ad60), (0x5d60c0, 0x5d60e0), (0x67a400, 0x690000)], sines=256)   # (the C library's acos and its error paths)
OPS = [k for k in os.environ.get('OPS', 'Touch,Touch,Quad').split(',') if k]
GLOBALS = 0x79223c   # (to 0x792390: the cars, the shapes, both shapes' places, the quad's corners)
SIZE = 0x792390 - GLOBALS
CAR1, CAR2, LIST1, LIST2, POINTS1, POINTS2, QUAD, WAY, P = (RAM + k * 0x100 for k in range(9))
NANS = [0x7fc00000, 0xffc00000, 0x7fa00000, 0x7f800000]
CRT_FLAG = 0xfb1330   # (set: acos past 1 returns its NaN without the math error path, whose start-up tables a bench does not have)


def f(r, nan, lo, hi):
    return r.choice(NANS) if r.random() < nan else fb(r.uniform(lo, hi))


def car(r, nan, cx, cy, cz):
    c = bytearray(r.bytes(0x60))
    x, y, z = cx + r.uniform(-1, 1), cy + r.uniform(-1, 1), cz + r.uniform(-1, 1)
    put(c, 4, 'III', f(r, nan, x, x), f(r, nan, y, y), f(r, nan, z, z))
    x2, y2, z2 = x + r.uniform(-2, 2), y + r.uniform(-2, 2), z + r.uniform(-2, 2)
    put(c, 0x3c, 'III', f(r, nan, x2, x2), f(r, nan, y2, y2), f(r, nan, z2, z2))
    flat = r.random() < 0.4
    put(c, 0x10, 'III', fb(0.0) if flat else f(r, nan, -3.2, 3.2), fb(0.0) if flat else f(r, nan, -3.2, 3.2), f(r, nan, -3.2, 3.2))
    return c


def points(r, nan, n, big):
    blob = bytearray(r.bytes(0x40))
    for k in range(n):
        angle = 2 * math.pi * k / max(n, 1) + r.uniform(-0.2, 0.2)
        put(blob, 8 * k, 'II', f(r, nan, big * math.cos(angle), big * math.cos(angle)), f(r, nan, big * math.sin(angle), big * math.sin(angle)))
    return blob


def touch(r, nan):
    n = r.deal([0, 1, 2, 3, 4, 4, 4, 5, 6])
    g = bytearray(r.bytes(SIZE))
    put(g, 0, 'III', CAR1, CAR2, LIST2)
    put(g, 0x7922dc - GLOBALS, 'I', LIST1)
    far = r.deal([0.0, 0.0, 0.0, 1.0, 3.0, 9.0])
    m = r.choice([n, n, n, max(n - 1, 0), 0])
    regions = {GLOBALS: g, CRT_FLAG: b'\1\0\0\0', CAR1: car(r, nan, 0, 0, 0), CAR2: car(r, nan, far, far / 2, -far),
               LIST1: put(bytearray(r.bytes(8)), 0, 'II', POINTS1, n), LIST2: put(bytearray(r.bytes(8)), 0, 'II', POINTS2, m),
               POINTS1: points(r, nan, n, r.uniform(0.5, 3)), POINTS2: points(r, nan, n, r.uniform(0.5, 3))}
    return Call(0x43a2b0, [], regions, ret='a')


def quad(r, nan):
    g = bytearray(r.bytes(SIZE))
    corners = bytearray()
    nx, ny = r.uniform(-1, 1), r.uniform(-1, 1)
    plane = lambda u, v: (u + nx * v, v * 0.5 + ny * u, u - v)   # noqa: E731
    for u, v in ((-1, -1), (1, -1), (1, 1), (-1, 1)):
        corners += struct.pack('<3I', *(f(r, nan, c, c) for c in plane(u * 2.0, v * 2.0)))
    at = [QUAD + 12 * k for k in range(4)]
    r.shuffle(at) if r.random() < 0.2 else None
    put(g, 0x79238c - GLOBALS, 'I', at[0])
    put(g, 0x792384 - GLOBALS, 'I', at[1])
    put(g, 0x792388 - GLOBALS, 'I', at[2])
    put(g, 0x792380 - GLOBALS, 'I', at[3])
    u, v = r.uniform(-1.9, 1.9), r.uniform(-1.9, 1.9)
    if r.random() < 0.3:
        u = r.choice([-1, 1]) * r.uniform(2.1, 5)
    i = plane(u, v)
    way = [r.uniform(-3, 3) for _ in range(3)]
    t = r.choice([0.0, 1.0, r.random(), r.random()])
    point = struct.pack('<3I', *(f(r, nan, i[k] - t * way[k], i[k] - t * way[k]) for k in range(3)))
    regions = {GLOBALS: g, CRT_FLAG: b'\1\0\0\0', QUAD: corners, WAY: struct.pack('<3I', *(f(r, nan, w, w) for w in way)), P: point}
    return Call(0x43aa10, [P, fb(t), WAY], regions, ret='a')


def case(r):
    nan = r.deal([0, 0, 0, 0, 0, 0.02])
    return touch(r, nan) if r.deal(OPS) == 'Touch' else quad(r, nan)


if __name__ == '__main__':
    B.main(case)
