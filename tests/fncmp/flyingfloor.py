# The flying rides' floor at a world position (src/game/world/flyingrides: 0x55cc00) against the original, over the surface height
# (Terrain_SurfaceHeight 0x452ae0 and what it reads: rideat.py's Surface world, whose cell the position falls in mostly): bounds about the
# position (inside, inside the 20000 margin, or past it on each side), grid sizes (0x7a0314, 0x7a0374) past the cell or not (clamping),
# positions left of 0 now and then, and 0-4 flying ride records (0x8791fc-0x879388) near the 10000 distance, far, or far enough for the
# squared distance to wrap.
# usage: flyingfloor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, put  # noqa: E402
import rideat  # noqa: E402  (its Surface world)

B = Bench([(0x55cc00, 0x55cd91), (0x67a830, 0x67a860), (0x452a60, 0x452ad7), (0x450ac0, 0x450b00), (0x53bf30, 0x53bfa0),
           (0x527e80, 0x527ed8), (0x527d60, 0x527e76), (0x4d99e0, 0x4d9a5b), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1),
           (0x452ae0, 0x452c97), (0x4527f0, 0x452a70)])
RIDES = RAM + 0x30000


def bounds(r, v):
    """(low, high) for a coordinate v: around it mostly, else with v inside the margin or past it on either side."""
    k = r.random()
    if k < 0.7:
        return v - r.randrange(0, 40000), v + r.randrange(1, 40000)
    if k < 0.85:
        low = v + r.choice([1, r.randrange(1, 20001), 20000, 20001, r.randrange(20001, 60000)])
        return low, low + r.randrange(1, 40000)
    high = v - r.choice([0, r.randrange(0, 20000), 19999, 20000, r.randrange(20000, 60000)])
    return high - r.randrange(1, 40000), high


def case(r):
    surface = rideat.surface(r)
    cx, cy = surface.args[0], surface.args[1]
    regions = surface.regions
    x, y = (c * 3000 + r.randrange(3000) for c in (cx, cy))
    if r.random() < 0.08:
        x = r.choice([-1, -2999, -3000, -r.randrange(1, 10000)])
    if r.random() < 0.08:
        y = r.choice([-1, -3000, -r.randrange(1, 10000)])
    (lx, hx), (ly, hy) = bounds(r, x), bounds(r, y)
    regions[0x7660b8] = struct.pack('<4i', hx, lx, hy, ly)
    regions[0x7a0314] = struct.pack('<i', r.choice([cx + 1, cx + 1 + r.randrange(10), cx, 1, 0]))
    regions[0x7a0374] = struct.pack('<i', r.choice([cy + 1, cy + 1 + r.randrange(10), cy, 1, 0]))
    table, rides = bytearray(4 * 100), bytearray(r.bytes(0x20 * 5))
    for i in range(r.choice([0, 0, 1, 2, 4])):
        k = r.random()
        if k < 0.6:   # about the 10000 distance (the distance in 32s, truncated, then << 5)
            d = r.choice([r.uniform(9000, 11000), 10016.0, 9984.0, r.uniform(0, 20000)])
            ox, oy = (int(d * c) for c in r.choice([(1, 0), (0, 1), (0.6, 0.8), (-0.8, 0.6), (0.7071, -0.7071)]))
        elif k < 0.85:
            ox, oy = r.randrange(-200000, 200000), r.randrange(-200000, 200000)
        else:   # (the square wraps)
            ox, oy = r.choice([0x7fffffff, -0x80000000, 1 << 21, 3 << 20]), r.randrange(-(1 << 22), 1 << 22)
        put(rides, 0x20 * i + 0x14, 'I', (x - ox) & 0xffffffff)
        put(rides, 0x20 * i + 0x1c, 'I', (y - oy) & 0xffffffff)
        put(table, 4 * r.randrange(1, 100), 'I', RIDES + 0x20 * i)
    regions.update({0x8791f8: table, RIDES: rides})
    return Call(0x55cc00, [x & 0xffffffff, y & 0xffffffff], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
