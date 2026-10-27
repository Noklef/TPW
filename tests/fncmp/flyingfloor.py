# The flying rides' floor at a world position (src/game/world/flyingrides: 0x55cc00) against the original, over the surface height
# (Terrain_SurfaceHeight 0x452ae0 and what it reads: rideat.py's Surface world, whose cell the position falls in mostly): bounds about the
# position (inside, inside the 20000 margin, or past it on each side), grid sizes (0x7a0314, 0x7a0374) past the cell or not (clamping),
# positions left of 0 now and then, and 0-4 flying ride records (0x8791fc-0x879388) near the 10000 distance, far, or far enough for the
# squared distance to wrap.
# Pick (the waypoint pick 0x55c350): see pick().
# usage: flyingfloor.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Floor,Pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, put  # noqa: E402
import rideat  # noqa: E402  (its Surface world)

B = Bench([(0x55cc00, 0x55cd91), (0x67a830, 0x67a860), (0x452a60, 0x452ad7), (0x450ac0, 0x450b00), (0x53bf30, 0x53bfa0),
           (0x527e80, 0x527ed8), (0x527d60, 0x527e76), (0x4d99e0, 0x4d9a5b), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1),
           (0x452ae0, 0x452c97), (0x4527f0, 0x452a70), (0x55c350, 0x55cbf3), (0x516330, 0x516380), (0x42cd40, 0x42cd7f)])
RIDES, REC = RAM + 0x30000, RAM + 0x31000
OPS = [k for k in os.environ.get('OPS', 'Floor,Pick').split(',') if k]


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
    if r.deal(OPS) == 'Pick':
        return pick(r)
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
        put(table, 4 * r.choice([r.randrange(1, 100), 1, 99]), 'I', RIDES + 0x20 * i)   # (the first and last indices often)
    regions.update({0x8791f8: table, RIDES: rides})
    return Call(0x55cc00, [x & 0xffffffff, y & 0xffffffff], regions, ret='a')


def pick(r):
    """0x55c350 (rec): Floor's world (rideat.py's Surface), the grid sizes such that every clamped cell is one of its (columns 0-0x80,
    rows 0-3; a size of 0, the cell -1, reads the world's cells before its first); the waypoint (+0x4c, +0x54) about the cell's position or a square's centre or edge, the last
    leg (to +0x58 / +0x60) along either axis, diagonal (|dx| = |dz|), none or any; here (+4, +0xc) the waypoint, a square away or any;
    the floor limit's +8 about the floors (never so high that +8 + 5000 wraps: the search would run ~2^32 / 5000 rounds); bounds about
    the waypoint as Floor has them; 0-2 flying rides near a step from it (FloorAt's 30000); the world's seed any."""
    c = rideat.surface(r)
    regions = c.regions
    cx, cy = c.args[0], c.args[1]
    x = cx * 3000 + r.choice([r.randrange(3000), 10000, 0, -1, 19999, 20000, -20000, -10000, r.randrange(-60000, 60000)])
    z = cy * 3000 + r.choice([r.randrange(3000), 10000, 0, -1, 19999, 20000, -20000, -10000, r.randrange(-60000, 60000)])
    if r.random() < 0.03:
        x = r.choice([0x7fffd000, -0x7fffd000, r.getrandbits(32) - (1 << 31)])
    k = r.random()
    if k < 0.35:
        ox, oz = x - r.choice([20000, -20000, 1, -1, r.randrange(-40000, 40000)]), z
    elif k < 0.7:
        ox, oz = x, z - r.choice([20000, -20000, 1, -1, r.randrange(-40000, 40000)])
    elif k < 0.8:
        d = r.choice([1, 20000, r.randrange(1, 40000)])
        ox, oz = x - d * r.choice([1, -1]), z - d * r.choice([1, -1])
    elif k < 0.9:
        ox, oz = x, z
    else:
        ox, oz = r.getrandbits(32), r.getrandbits(32)
    hx, hz = r.choice([(x, z), (x + 20000, z), (x, z - 20000), (cx * 3000 + r.randrange(-30000, 30000), cy * 3000 + r.randrange(-6000, 12000)),
                       (r.getrandbits(32), r.getrandbits(32))])
    rec = bytearray(r.bytes(0x80))
    put(rec, 4, 'I', hx & 0xffffffff)
    put(rec, 0xc, 'I', hz & 0xffffffff)
    put(rec, 8, 'i', r.choice([0, -5000, 5000, 15000, 25000, 95000, 100000, r.randrange(-60000, 120000), r.randrange(-2000000, 0)]))
    for off, v in ((0x4c, x), (0x54, z), (0x58, ox), (0x60, oz)):
        put(rec, off, 'I', v & 0xffffffff)
    (lx, ux), (lz, uz) = bounds(r, x if abs(x) < 1 << 30 else cx * 3000), bounds(r, z)   # (a far waypoint: outside them)
    regions[0x7660b8] = struct.pack('<4i', ux, lx, uz, lz)
    regions[0x7a0314] = struct.pack('<i', r.choice([cx + 1, 0x81, r.randrange(1, 0x82), 1, 1]))
    regions[0x7a0374] = struct.pack('<i', r.choice([cy + 1, 4, r.randrange(1, 5), 1]))
    table, rides = bytearray(4 * 100), bytearray(r.bytes(0x20 * 2))
    for i in range(r.choice([0, 0, 0, 1, 2])):
        ex, ez = r.choice([(20000, 0), (-20000, 0), (0, 20000), (0, -20000), (0, 0)])
        put(rides, 0x20 * i + 0x14, 'I', (x + ex + r.randrange(-12000, 12000)) & 0xffffffff)
        put(rides, 0x20 * i + 0x1c, 'I', (z + ez + r.randrange(-12000, 12000)) & 0xffffffff)
        put(table, 4 * r.choice([r.randrange(1, 100), 1, 99]), 'I', RIDES + 0x20 * i)
    regions.update({0x8791f8: table, RIDES: rides, REC: rec, rideat.WORLD + 0x1da708: r.bytes(4)})
    return Call(0x55c350, [REC], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
