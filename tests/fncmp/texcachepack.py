# The texture cache's items packed into an area (src/game/gfx/texcache: 0x58eca0) against the original: 0-12 items (now and then 140 tiny ones, which reach the 0x7f rows' stop)
# of sizes a few values, areas a few values, the cache's flags 1 and 2 and mask, the driver filter on or off. The free rectangles live on the stack, so the real sort runs.
# usage: texcachepack.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x57ced0, 0x57d010), (0x57d090, 0x57d0d0), (0x58cb40, 0x58cbb0), (0x58eca0, 0x58f1a0), (0x590fb0, 0x590fe0)])
CACHE, AREA, LIST, OUT, ITEMS = RAM, RAM + 0x2000, RAM + 0x2100, RAM + 0x3000, RAM + 0x4000


def item(r, sizes):
    buf = bytearray(r.bytes(0x40))
    flags = r.getrandbits(32) & ~0x30030
    put(buf, 8, 'I', flags | r.choice([0, 0x10, 0x20, 0x30, 0x30]) | r.choice([0, 0x30000, 0x10000, 0x20000]))
    put(buf, 0x14, 'hh', r.choice(sizes), r.choice(sizes))
    put(buf, 0x30, 'f', r.random())
    put(buf, 0x38, 'i', r.randrange(-3, 40))
    return buf


def halved_key(buf):
    flags, width, height, lod = struct.unpack_from('<I', buf, 8)[0], *struct.unpack_from('<hh', buf, 0x14), struct.unpack_from('<f', buf, 0x30)[0]
    key = ((int((1.5 - lod) * 2047.0) & 0xfff) << 8) + ((width + 1) >> 1) << 8
    key += (height + 1) >> 1
    key |= (0x20000000 if flags & 0x4000 else 0) | (0x40000000 if flags & 0x80000 else 0)
    key &= 0xffffffff
    return key - (1 << 32) if key >> 31 else key


def case(r):
    many = r.randrange(8) == 0
    n = 140 if many else r.choice([0, 1, 2, 3, 5, 8, 12])
    sizes = [1, 2, 3] if many else [1, 4, 8, 16, 16, 32, 32, 64, 128, 256, 300, -1, r.randrange(-40, 70)]
    w, h = (256, 256) if many else (r.choice([0, 8, 16, 64, 128, 256, 256]), r.choice([0, 8, 16, 64, 128, 256, 256]))
    cache = bytearray(r.bytes(0x1930))
    put(cache, 0x18cc, 'I', r.getrandbits(32) & ~3 | r.choice([0, 1, 2, 3, 1, 3]))
    put(cache, 0x18e4, 'I', r.choice([0, 1, 3, 7]))
    put(cache, 0x18e8, 'I', r.choice([0, 0, 1, 0x100]))
    area = bytearray(r.bytes(0x10))
    put(area, 8, 'ii', w, h)
    many_items = [item(r, sizes) for i in range(n)]
    for before, after in zip(many_items, many_items[1:]):
        if r.randrange(3) == 0:   # (a key tying with the one before's once halved)
            put(after, 0x38, 'i', halved_key(before))
    items = b''.join(bytes(i) for i in many_items)
    order = struct.pack('<%dI' % (n + 1), *([ITEMS + 0x40 * i for i in range(n)] + [0]))
    regions = {CACHE: cache, AREA: area, LIST: order + r.bytes(4), OUT: bytearray(0x14 * 0x90), 0x8c4b68: r.bytes(8),
               0x8bd520: r.bytes(4) + struct.pack('<I', r.choice([1, 1, 0, 4])), 0x702434: struct.pack('<i', r.choice([2, 8, 8, 32]))}
    if n:
        regions[ITEMS] = items
    return Call(0x58eca0, [CACHE, LIST, OUT, OUT + 0x14 * 0x8f, AREA, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
