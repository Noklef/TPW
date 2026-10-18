# A nav's path to a point (src/game/world/nav: 0x50f8e0, 0x510100 that takes the point's two parts) and a moving person's destination
# (src/game/world/person: 0x4fa5f0) against the original. The nav's: routesearch.py's box of cells, the nav's position in one of its cells
# (at times with a wild top byte), the target in the box, at the start or in the ring; the route at 0x7cf108 random before the search, the
# counter 0x7cdb98, prepending 0, 1 or other. The person's: objectblocks.py's object and block values (the limit +0x198 about the counter),
# the destination's cell the nav's (its route then that one cell, needing no map).
# usage: navpath.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import objectblocks  # noqa: E402
import routesearch  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402

RANGES = []
for a, b in sorted([(0x50f8e0, 0x50fd2f), (0x510100, 0x51014d), (0x4fa5f0, 0x4fa669), (0x50cd80, 0x50cda8), (0x4d8c50, 0x4d8c5c)]
                   + routesearch.RANGES + objectblocks.RANGES):  # (the two benches' ranges overlap)
    if RANGES and a <= RANGES[-1][1]:
        RANGES[-1] = (RANGES[-1][0], max(RANGES[-1][1], b))
    else:
        RANGES.append((a, b))
B = Bench(RANGES)
NAV = RAM + 0x48000
TARGET = RAM + 0x48800


def fixed(cell, r):
    """A 16.16 coordinate in the cell: its byte 2 the cell, at times a wild top byte."""
    return (r.getrandbits(8) << 24 if r.random() < 0.2 else 0) | cell << 16 | r.getrandbits(16)


def common(regions, r, counter):
    regions[0x7cf108] = r.bytes(0x206)
    g = bytearray(r.bytes(0x34))
    put(g, 0x7cf3c8 - routesearch.GLOBALS, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions[routesearch.GLOBALS] = bytes(g)
    regions[0x7cdb98] = struct.pack('<I', counter & 0xffffffff)


def nav_case(r, op):
    mode = r.choice([0, 1, 2, 2, 2, 3])
    regions, box, ring, bx, by, size = routesearch.boxed(r, mode)
    nav = bytearray(r.bytes(0x100))
    sx, sy = r.choice(sorted(box))
    put(nav, 8, 'II', fixed(sx, r), fixed(sy, r))
    put(nav, 0xb4, 'I', mode)
    tx, ty = r.choice(sorted(box) * 6 + [(sx, sy)] * 2 + sorted(ring))
    target = (fixed(tx, r), fixed(ty, r))
    regions[NAV] = nav
    common(regions, r, r.choice([0, 0xffffffff, r.getrandbits(32)]))
    if op == 0x510100:
        return Call(op, [NAV, *target], regions, conv='t', ret='a')
    regions[TARGET] = struct.pack('<II', *target)
    return Call(op, [NAV, TARGET, r.choice([0, 1, 1, 1, 2, r.getrandbits(32)])], regions, conv='t', ret='a')


def person_case(r):
    c = objectblocks.case(r)
    regions = dict(c.regions)
    obj = regions[RAM]
    limit = struct.unpack_from('<I', obj, 0x198)[0]
    x, y = r.getrandbits(16), r.getrandbits(16)
    sx, sy = (((v ^ 0x8000) - 0x8000) << 8 >> 16 & 0xff for v in (x, y))  # (the destination's cell: (short)x << 8, byte 2)
    put(obj, 0xd4 + 8, 'II', fixed(sx, r), fixed(sy, r))
    put(obj, 0xd4 + 0xb4, 'I', r.choice([0, 1, 2, 3]))
    common(regions, r, r.choice([limit - 2, limit - 1, limit, r.getrandbits(32)]))
    return Call(0x4fa5f0, [RAM, x | r.getrandbits(16) << 16, y | r.getrandbits(16) << 16], regions, conv='t', ret='a')


def case(r):
    op = r.deal(OPS or [0x50f8e0, 0x510100, 0x4fa5f0])
    return person_case(r) if op == 0x4fa5f0 else nav_case(r, op)


OPS = [int(k, 16) for k in os.environ.get('OPS', '').split(',') if k]


if __name__ == '__main__':
    B.main(case)
