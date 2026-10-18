# The texture cache's texture placement (src/game/gfx/texcache: 0x590350) against the original: driver kinds 4 and others; the
# texture's flags 0x80000000 (sizes past the cut or not, odd excesses), 0x10000 / 0x20000, 0x4000000, 0x400 with +0x3c 0 or not; null
# (the place's +0x10) or given; the texel offsets 0x8c1628-0x8c163c and the cache's scale any floats (r.f(); NaNs, infinities and extremes
# in wild); the surface's sizes 0-0x100 (0: infinities) and its slots 7 and 2 recorders.
# usage: texcacheplace.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, vtable  # noqa: E402

B = Bench([(0x590350, 0x590692), (0x590cb0, 0x590cf0)])
CACHE, ENTRY, SURF, VT, PLACE, TEX, OBJ = RAM, RAM + 0x2000, RAM + 0x2100, RAM + 0x2200, RAM + 0x2300, RAM + 0x2400, RAM + 0x2500


def size(r):
    return r.choice([1, 2, 8, 16, 64, 100, 0x100, r.randrange(-50, 300)])


def case(r):
    log = log_region()
    cache = bytearray(r.bytes(0x1904))
    put(cache, 0x1900, 'f', r.f())
    surface = bytearray(struct.pack('<I', VT) + r.bytes(0x1c))
    put(surface, 8, 'iii', r.choice([0, 1, 64, 0x100, r.randrange(0x101)]), r.choice([0, 1, 64, 0x100, r.randrange(0x101)]), r.choice([0, 0x100, r.randrange(0x400)]))
    texture = bytearray(r.bytes(0x48))
    flags = r.getrandbits(32) & ~0x84030400
    for bit in (0x80000000, 0x10000, 0x20000, 0x4000000, 0x400):
        if r.random() < 0.45:
            flags |= bit
    put(texture, 8, 'I', flags)
    w, h = size(r), size(r)
    put(texture, 0x10, 'hhhh', r.choice([w, w - 1, w - 3, w + 2, size(r)]), r.choice([h, h - 1, h - 2, h + 5, size(r)]), w, h)
    put(texture, 0x3c, 'I', r.choice([0, 0, r.getrandbits(32)]))
    given = r.random() < 0.8
    place = bytearray(r.bytes(0x14))
    put(place, 0, 'ii', r.randrange(-10, 300), r.randrange(-10, 300))
    put(place, 0x10, 'I', TEX)
    offsets = struct.pack('<6f', *[r.choice([0.0, 0.5, 1.0, r.f()]) for _ in range(6)])
    regions = {CACHE: cache, ENTRY: struct.pack('<I', SURF) + r.bytes(0x1c), SURF: surface, VT: vtable({7: 3}, 8), PLACE: place, TEX: texture, LOG: log,
               OBJ: r.bytes(8), 0x87a248: struct.pack('<I', OBJ), 0x8bd524: struct.pack('<I', r.choice([4, 4, 1, 1, 0, 3])), 0x8bcc40: r.bytes(4),
               0x8c1628: offsets}
    return Call(0x590350, [CACHE, ENTRY, PLACE, TEX if given else 0], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
