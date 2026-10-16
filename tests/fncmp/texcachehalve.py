# The texture cache's 2 x 2 box filter (src/game/gfx/texcache: 0x58b890) against the original:
# both pixel formats and others, widths and heights from 0 to 5, pitches that leave gaps, overlap
# or step backwards, any pixels.
# usage: texcachehalve.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x58b890, 0x58baeb)])
CACHE, DEST, SRC, OUT, IN = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x1000, RAM + 0x3000


def surface(r, pixels, w, h, pitch):
    s = bytearray(r.bytes(0x30))
    put(s, 8, 'II', h, w)
    put(s, 0x10, 'i', pitch)
    put(s, 0x24, 'I', pixels)
    return s


def case(r):
    fmt = r.choice([2, 3, 3, 2, 3, 2, 1, 0, 4])
    bpp = 2 if fmt == 2 else 4
    w, h = r.randrange(6), r.randrange(5)
    cache = bytearray(r.bytes(0x40))
    put(cache, 0x30, 'I', fmt)
    exact = r.random() < 0.6
    dpitch = bpp * w + (r.randrange(0, 12) if exact else r.randrange(-8, 40))
    spitch = 2 * bpp * w + (r.randrange(0, 12) if exact else r.randrange(-8, 40))
    regions = {CACHE: cache, DEST: surface(r, OUT + 0x400, w, h, dpitch), SRC: surface(r, IN + 0x800, 2 * w, 2 * h, spitch),
               OUT: r.bytes(0x800), IN: r.bytes(0x1800)}
    return Call(0x58b890, [CACHE, DEST, SRC], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
