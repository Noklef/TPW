# The dynamic face queue writers and the tag sort of src/game/render/dynfaces (0x53c8d0, 0x53c9c0, 0x53c1f0) against the originals: counts of faces from none to full (the 0x806 limit too), tags
# including 1 with the flag byte either way, the square's arguments and the polygon's four corners / uvs / colours; tag words of few or all 32 tags to sort,
# from none (which the original's unsigned compares send to the partition) to 300 (runs of all sizes). Grid (0x53cab0): up to 4 x 4 quads
# (none and negative counts too) from rows and columns 0 to 17 of the 19-wide position and colour tables (only the rows read laid out),
# float offsets and table values, the queue empty, part full, just full or past it.
# usage: dynfaces.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Square,Poly,Sort,Grid)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Square': 0x53c8d0, 'Poly': 0x53c9c0, 'Sort': 0x53c1f0, 'Grid': 0x53cab0}
B = Bench([(0x53c8d0, 0x53c9c0), (0x53c9c0, 0x53cab0), (0x5da3c0, 0x5da3c1), (0x53c1f0, 0x53c3e4), (0x53cab0, 0x53cd9a)])


def grid(r):
    rows, cols = r.choice([1, 2, 3, 4, 4, 0, -1]), r.choice([1, 2, 3, 4, 4, 0, -2])
    row, col = r.choice([0, 3, 0xa, r.randrange(18)]), r.choice([0, 5, 0x10, 17, r.randrange(18)])
    faces = max(rows, 0) * max(cols, 0)
    n = r.choice([0, 3, 0x806 - faces // 2, 0x806, 0x807])
    span = 12 * 19 * (max(rows, 0) + 2)
    pos, col_bytes = bytearray(span), bytearray(span)
    scale = r.choice([1, 100, 5000])
    for k in range(0, span, 4):
        struct.pack_into('<I', pos, k, r.f(scale))
        struct.pack_into('<I', col_bytes, k, r.f(1))
    regions = {0x874180 + 12 * 19 * row: pos, 0x870ed8 + 12 * 19 * row: col_bytes, 0x871d18: r.bytes(4 * 16 + 4),
               0x871ee0: struct.pack('<I', n)}
    if n < 0x806:
        regions[0x820b00 + 0xa0 * n] = r.bytes(0xa0 * min(faces, 0x806 - n) or 4)
        regions[0x872168 + 4 * n] = r.bytes(4 * min(faces, 0x806 - n) or 4)
    args = [r.f(scale), r.f(scale), r.f(scale), rows & 0xffffffff, cols & 0xffffffff, row, col, 0x10, 0x13]
    return Call(CALLS['Grid'], args, regions, ret='v')


def case(r):
    name = r.deal(OPS)
    if name == 'Grid':
        return grid(r)
    if name == 'Sort':
        n = r.choice([0, 1, 2, 3, 6, 7, 8, 9, 15, 40, 300]) if r.random() < 0.6 else r.randrange(0, 120)
        kinds = r.choice([1, 2, 3, 5, 32])
        words = [r.getrandbits(27) << 5 | (r.randrange(kinds) if kinds < 32 else r.getrandbits(5)) for _ in range(n + 2)]
        if r.random() < 0.15:  # already sorted or reversed
            words[1:n + 1] = sorted(words[1:n + 1], key=lambda w: w & 0x1f, reverse=r.random() < 0.5)
        return Call(CALLS[name], [RAM + 0x1000, n], {RAM + 0x1000: struct.pack('<%dI' % len(words), *words)}, ret='v')
    n = r.choice([0, 1, 2, 5, 0x805, 0x806, 0x807])
    base = 0x820b00 + 0xa0 * min(n, 6)
    regions = {0x820b00: r.bytes(0xa0 * 9), 0x872168: r.bytes(0x40), 0x871ee0: struct.pack('<I', min(n, 6) if n < 0x806 else n), 0x763c98: bytes([r.choice([0, 1])]) + bytes(3),
               0x874fc4: struct.pack('<I', r.choice([0, 0, 1, 5])), RAM: r.bytes(0x100), RAM + 0x100: r.bytes(0x100), RAM + 0x200: r.bytes(0x100)}
    tag = r.choice([0, 1, 1, 2, 0x1f, 0x20, 0xff, r.getrandbits(32)])
    if name == 'Square':
        w = lambda: r.getrandbits(32)  # noqa: E731
        return Call(CALLS[name], [w(), w(), w(), w(), tag, w(), w(), w(), w(), w(), w()], regions, ret='v')
    return Call(CALLS[name], [RAM, tag, RAM + 0x100, RAM + 0x200, r.getrandbits(32)], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
