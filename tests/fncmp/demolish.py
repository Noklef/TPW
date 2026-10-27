# Things taken off the map by area (src/game/world/demolish: Map_DemolishRect 0x52bdd0) against the original: a rectangle of 1-5 by 1-5
# cells (corners in either order; near the map's edges now and then), the map [0x7cf83c]'s cells around it laid out row by row with a ring
# three cells out that stops every line (type 0, unless off the map); the cells' types mostly 4, 9, 10 (the lines) and 1, their owner
# words +0x10 one of two mostly, the flag 0x10 (+0xe) now and then. The demolish 0x527ee0 (not ported yet) a cdecl recorder (k 2 of 3) and
# agent B's unported 0x5367a0 a thiscall recorder (k 28 of 2), driver.cpp's kDemolishEnv.
# usage: demolish.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

B = Bench([(0x52bdd0, 0x52c148), (0x535db0, 0x535de0), (0x536310, 0x536330), (0x4d8300, 0x4d8340), (0x5da3c0, 0x5da3c1)],
          hooks={0x527ee0: _recorder(2, 3, cdecl=True), 0x5367a0: _recorder(28, 2)})
MAP = RAM + 0x100000
TYPES = [4, 4, 9, 10, 4, 9, 10, 1, 1, 0, 8, 11, 2, 3, 0x104]


def cell_at(x, y):
    return MAP + 0x294 + ((((y << 7) + 1 + x) & 0xffff) * 0x44)


def coord(r, span):
    return r.choice([0, 1, 2, 3, 127 - span, 126 - span, r.randrange(0, 128 - span), r.randrange(0, 128 - span)])


def case(r):
    w, h = r.randrange(5), r.randrange(5)
    x, y = coord(r, w), coord(r, h)
    x0, x1 = (x, x + w) if r.random() < 0.5 else (x + w, x)
    y0, y1 = (y, y + h) if r.random() < 0.5 else (y + h, y)
    owners = [r.getrandbits(16), r.getrandbits(16)]
    linear = r.choice(['row', 'col', None])   # (whole lines of one owner along the rows or columns, or owners at random)
    regions = {LOG: log_region(), 0x7cf83c: struct.pack('<I', MAP)}
    xlo, xhi, ylo, yhi = max(0, x - 3), min(127, x + w + 3), max(0, y - 3), min(127, y + h + 3)
    for row in range(ylo, yhi + 1):
        cells = bytearray(r.bytes(0x44 * (xhi - xlo + 1)))
        for col in range(xlo, xhi + 1):
            c = 0x44 * (col - xlo)
            ring = col in (x - 3, x + w + 3) or row in (y - 3, y + h + 3)
            kind = 0 if ring else r.choice(TYPES)
            put(cells, c + 8, 'I', kind)
            flags = r.getrandbits(16) & ~0x10
            if r.random() < 0.1:
                flags |= 0x10
            put(cells, c + 0xe, 'H', flags)
            owner = owners[{'row': row, 'col': col}[linear] & 1] if linear and r.random() < 0.9 else r.choice(owners)
            put(cells, c + 0x10, 'H', owner)
        regions[cell_at(xlo, row)] = cells
    return Call(0x52bdd0, [x0 & 0xffffffff, y0 & 0xffffffff, x1, y1], regions, ret='b')


if __name__ == '__main__':
    B.main(case)
