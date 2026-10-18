# The strip animation (src/game/model/stripanim: 0x4543c0, 0x4547c0, 0x454190) against the originals, dealt in turn (Apply twice as
# often): an object (+4 flags, +0xb0 -> count, +0xb4 -> shape) with grouped vertices and texture coordinates, t across the ramp (and
# wild), counts from none up.
# FitGround: strips (count, divisors 0-8, ends of random halves mostly up to the divisors) on a footprint of up to 6 by 6 cells at a cell
# 0-10, turned by the holder's right angle (now and then another), over a grid 24 cells wide of heights about -50..50 (now and then
# infinite or NaN); sometimes no map or grid, or flag 2 clear.
# usage: stripanim.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, fb, put  # noqa: E402

B = Bench([(0x454050, 0x45453b), (0x4547c0, 0x4547e7)])
OBJECT, SHAPE, CHAIN, VERTS, COORDS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x2000


HOLDER, MODEL, XF, STRIPS, ENDS, FSHAPE, FVERTS, MAP, GRID, HEIGHTS = (RAM + 0x10000 * i for i in range(1, 11))
GW = 24


def height(r):
    if r.random() < 0.03:
        return r.choice([float('nan'), float('inf'), float('-inf'), 3e38])
    return float(r.randrange(-50, 51)) if r.random() < 0.3 else r.uniform(-50, 50)


def fit_case(r):
    n = r.choice([1, 2, 3, 6, r.randrange(-1, 7)])
    obj = bytearray(r.bytes(0xc8))
    put(obj, 4, 'I', r.getrandbits(32) | 2 if r.random() < 0.95 else r.getrandbits(32) & ~2)
    put(obj, 8, 'I', HOLDER)
    put(obj, 0xb0, '2I', STRIPS, FSHAPE)
    x0, y0 = r.randrange(11), r.randrange(11)
    put(obj, 0xc0, '2I', x0 << 16 | y0, (x0 + r.randrange(-1, 7)) % 0x10000 << 16 | (y0 + r.randrange(-1, 7)) % 0x10000)
    holder = put(bytearray(r.bytes(0x14)), 4, 'I', MODEL)
    put(holder, 0x10, 'i', r.choice([0, 90, 180, 270] * 6 + [r.randrange(-400, 400)]))
    s4, s8 = r.choice([1, 2, 4, 8, r.randrange(9)]), r.choice([1, 2, 4, 8, r.randrange(9)])
    ends = bytearray()
    for _ in range(2 * max(n, 0)):
        hi, lo = (r.randrange(0, 2 * max(s, 1) + 1) if r.random() < 0.2 else r.randrange(0, max(s, 1) + 1) for s in (s4, s8))
        ends += struct.pack('<I', hi << 16 | lo)
    grid = put(bytearray(r.bytes(0x30)), 0x18, 'I', GW)
    put(grid, 0x28, 'I', HEIGHTS)
    heights = bytearray(4 * (GW + 1) * 25)
    for i in range(len(heights) // 4):
        put(heights, 4 * i, 'f', height(r))
    world = put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID if r.random() < 0.97 else 0)
    regions = {OBJECT: obj, HOLDER: holder, MODEL: put(bytearray(r.bytes(0x7c)), 0x78, 'I', XF), XF: r.floats(r.bytes(0x48), [0x44], 50),
               STRIPS: put(bytearray(0x10), 0, '4I', n % (1 << 32), s4, s8, ENDS), ENDS: ends or bytearray(4), FSHAPE: put(r.bytes(0x14), 0x10, 'I', FVERTS),
               FVERTS: r.bytes(0x30 * max(n, 1) + 0x10), MAP: world, GRID: grid, HEIGHTS: heights,
               0x7a0854: struct.pack('<I', MAP if r.random() < 0.97 else 0)}
    return Call(0x454190, [OBJECT], regions)


def case(r):
    op = r.deal(['apply', 'apply', 'shrink', 'fit'])
    if op == 'fit':
        return fit_case(r)
    ram = bytearray(r.bytes(0x3000))
    n = r.choice([0, 1, 2, 3, 8, r.randrange(-3, 40)])
    put(ram, 4, 'I', r.getrandbits(32) | r.choice([2, 2, 2, 0]) if r.random() < 0.9 else r.getrandbits(32) & ~2)
    put(ram, 0xb0, 'I', CHAIN + 0x80)
    put(ram, 0x280, 'i', n)
    put(ram, 0xb4, 'I', SHAPE)
    put(ram, 0x110, 'I', VERTS)
    put(ram, 0x124, 'I', CHAIN)          # holder: +4 model
    put(ram, 0x204, 'I', CHAIN + 0x20)   # model: +0x78 root
    put(ram, 0x298, 'I', CHAIN + 0x40)
    put(ram, 0x2a8, 'I', COORDS)         # root: +0x68 coordinates
    r.floats(ram, range(VERTS - RAM, VERTS - RAM + 0x30 * 40, 4), r.choice([1, 10, 100, None]))
    t = r.choice([r.f(2), r.f(), fb(r.uniform(0, 1.25)), fb(0.25), fb(1.0), fb(0.0), r.f()])
    if op == 'shrink':  # StartShrink: strips or none, flags 0x20 / 0x40 / 0x80 either way
        put(ram, 0xb0, 'I', r.choice([CHAIN + 0x80, 0]))
        return Call(0x4547c0, [OBJECT], {RAM: ram})
    return Call(0x4543c0, [OBJECT, t], {RAM: ram})


if __name__ == '__main__':
    B.main(case)
