# A polygon clipped to a rectangle (src/game/math/rectclip: Polygon_ClipToRect 0x56c5a0) against the original; Segment_AtX and Segment_AtZ run as they are. The fixture: convex polygons of up to 8 points
# about the rectangle, or any 4 points, on its edges now and then (a tie), NaNs in the small ones, the limits swapped now and then. (More points overrun the 16 of its frame's buffer, in the original its
# own count at the end of it: a polygon of 4 points at worst, of any size if convex, stays inside.)
# usage: rectclip.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x56c5a0, 0x56d600)])
NANS = [0x7fc12345, 0x7f812345, 0xffc00000]


def fbits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def case(r):
    self = bytearray(r.bytes(0x100))
    x0, x1 = sorted(r.uniform(-100, 100) for _ in range(2))
    z0, z1 = sorted(r.uniform(-100, 100) for _ in range(2))
    if r.random() < 0.05:
        x0, x1 = x1, x0
    if r.random() < 0.05:
        z0, z1 = z1, z0
    if r.random() < 0.7:
        n = r.randrange(0, 9)
        angles = sorted(r.uniform(0, 2 * math.pi) for _ in range(n))
        rx, rz = r.uniform(1, 150), r.uniform(1, 150)
        cx, cz = r.uniform(-100, 100), r.uniform(-100, 100)
        points = [(cx + rx * math.cos(a), r.uniform(-50, 50), cz + rz * math.sin(a)) for a in angles]
        bad = False
    else:
        n = r.randrange(0, 5)
        points = [(r.uniform(-150, 150), r.uniform(-50, 50), r.uniform(-150, 150)) for _ in range(n)]
        bad = r.random() < 0.3
    for i, (x, y, z) in enumerate(points):
        if r.random() < 0.1:
            x = r.choice([x0, x1])
        if r.random() < 0.1:
            z = r.choice([z0, z1])
        words = [fbits(x), fbits(y), fbits(z)]
        if bad and r.random() < 0.3:
            words[r.randrange(3)] = r.choice(NANS)
        put(self, 12 * i, '3I', *words)
    put(self, 0xd0, 'I', n)
    return Call(0x56c5a0, [RAM, fbits(x0), fbits(x1), fbits(z0), fbits(z1)], {RAM: self}, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
