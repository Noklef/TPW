# The GUI library's circle, ellipse and polygon (src/game/ui/guishape: 0x65a9f0, 0x66c01a-0x66c7b0) against the originals: a shape of random bytes
# (now and then a centre and radii near each other, or edge values; the ellipse's radius across 0 now and then: a fault in both), the words given
# random, small or edge values (the upper halves random: only the low words count); a polygon's points (0 to 9, near each other or wild; a block
# or none; a negative count for the offset) and the points array its constructor copies; the deleting destructors' flags any.
# usage: guishape.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Base,Circle,CircleWith,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap  # noqa: E402

CALLS = {'Base': 0x65a9f0, 'Circle': 0x66c01a, 'CircleWith': 0x66c054, 'CircleContains': 0x66c0c4, 'CircleOffset': 0x66c125,
         'Ellipse': 0x66c163, 'EllipseWith': 0x66c1a6, 'EllipseContains': 0x66c22c, 'EllipseOffset': 0x66c2a1, 'CircleDelete': 0x66c760,
         'CircleDtor': 0x66c790, 'Polygon': 0x66c2df, 'Triangle': 0x66c311, 'Quad': 0x66c3cb, 'Points': 0x66c4a1, 'PolygonDtor': 0x66c568,
         'PolygonContains': 0x66c5a4, 'PolygonOffset': 0x66c6c3, 'PolygonDelete': 0x66c7b0}
B = Bench([(0x65a9d0, 0x65aa1b), (0x66bff7, 0x66c01a), (0x66c01a, 0x66c7dc), (0x67acb0, 0x67acb1)])
SHAPE, SOURCE, BLOCK = RAM, RAM + 0x100, RAM + 0x200


def word(r):
    w = r.choice([0, 1, 2, 5, 100, 0x7fff, 0x8000, 0xffff, 0xfffe, r.randrange(-40, 40) & 0xffff, r.getrandbits(16)])
    return w | (r.getrandbits(16) << 16 if r.random() < 0.5 else 0)


def near(r, at, spread):
    return (at + r.randrange(-spread, spread + 1)) & 0xffff


def high(r, w):
    return w | (r.getrandbits(16) << 16)


def points(r, n):
    if r.random() < 0.15:
        return struct.pack('<%dH' % (2 * n), *[r.getrandbits(16) for _ in range(2 * n)])
    return struct.pack('<%dh' % (2 * n), *[r.randrange(-30, 31) for _ in range(2 * n)])


def case(r):
    op = r.deal(OPS)
    address = CALLS[op]
    shape = bytearray(r.bytes(0x10))
    if r.random() < 0.7:
        struct.pack_into('<hhhh', shape, 4, r.randrange(-30, 31), r.randrange(-30, 31), r.randrange(-3, 40), r.randrange(-3, 40))
    regions = {SHAPE: shape, HEAP: heap()}
    if op.startswith('Polygon') and op != 'Polygon':
        n = r.choice([0, 1, 2, 3, 3, 4, 5, 6, 9])
        if op == 'PolygonOffset' and r.random() < 0.1:
            n = r.choice([-1, -3, -0x8000])
        struct.pack_into('<h', shape, 4, n)
        struct.pack_into('<I', shape, 8, BLOCK if r.random() < 0.9 else 0)
        regions[BLOCK] = points(r, max(n, 0)) + r.bytes(8)
    if op in ('Base', 'CircleDelete', 'PolygonDelete'):
        return Call(address, [SHAPE, r.choice([0, 1, 2, 3, r.getrandbits(32)])], regions, conv='t', ret='a')
    if op in ('Circle', 'Ellipse', 'Polygon', 'CircleDtor', 'PolygonDtor'):
        return Call(address, [SHAPE], regions, conv='t', ret='a')
    if op in ('CircleWith', 'EllipseWith', 'Triangle', 'Quad'):
        n = {'CircleWith': 3, 'EllipseWith': 4, 'Triangle': 6, 'Quad': 8}[op]
        return Call(address, [SHAPE] + [word(r) for _ in range(n)], regions, conv='t', ret='a')
    if op == 'Points':
        n = r.choice([0, 1, 2, 3, 4, 7, 9, -1, 0x8000 | r.getrandbits(4)]) if r.random() < 0.9 else r.getrandbits(4)
        regions[SOURCE] = points(r, 16)
        return Call(address, [SHAPE, high(r, n & 0xffff) if r.random() < 0.5 else n & 0xffff, SOURCE], regions, conv='t', ret='a')
    if op.endswith('Offset'):
        return Call(address, [SHAPE, word(r), word(r)], regions, conv='t')
    # (Contains: a point near the centre, or about the polygon's points, or anywhere)
    if op == 'EllipseContains' and struct.unpack_from('<h', shape, 8)[0] == 0 and r.random() < 0.9:
        struct.pack_into('<h', shape, 8, r.choice([1, -1, 7, 20]))
    cx, cy = struct.unpack_from('<hh', shape, 4) if not op.startswith('Polygon') else (0, 0)
    spread = r.choice([3, 30, 60])
    x, y = near(r, cx, spread), near(r, cy, spread)
    if r.random() < 0.15:
        x, y = word(r) & 0xffff, word(r) & 0xffff
    ret = 'a' if op == 'EllipseContains' else 'w'
    return Call(address, [SHAPE, high(r, x) if r.random() < 0.5 else x, high(r, y) if r.random() < 0.5 else y], regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
