# The texture cache entry's score (src/game/gfx/texcache: 0x591010) against the original: lists of 0-5 items (flags 0x4000,
# 0x40000000, 0x10000000, 0x30 bits and the driver filter's 0x30000 with 0x8bd524 = 1, on or off; widths around 0x702564's 8;
# sizes negative now and then; the weight past the virtual base at offsets 0-0x20), the surface's slot 3 (a recorder) answering an
# area (any now and then), the entry's generation 0-20 behind 0x8bcc40 (or ahead).
# usage: texcachescore.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x591010, 0x59118c), (0x590fb0, 0x590fcb), (0x58b330, 0x58b344)])
ENTRY, SURF, VT, VB, ITEMS = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x300, RAM + 0x1000


def case(r):
    log = log_region()
    off = r.choice([0, 0x10, 0x10, 0x20])
    n = r.choice([0, 1, 2, 3, 5])
    items = bytearray(r.bytes(0x40 * 6))
    for i in range(n):
        o = 0x40 * i
        put(items, o, 'II', VB, ITEMS + 0x40 * (i + 1) if i + 1 < n else 0)
        flags = r.getrandbits(32) & ~0x50034030
        for bit in (0x4000, 0x40000000, 0x10000000, 0x10, 0x20, 0x10000, 0x20000):
            if r.random() < 0.4:
                flags |= bit
        put(items, o + 8, 'I', flags)
        put(items, o + 0x14, 'hh', r.choice([7, 8, 9, 16, 1, 0, r.randrange(-300, 300)]), r.choice([0, 1, 8, 64, r.randrange(-300, 300)]))
        put(items, o + off + 0x10, 'i', r.choice([0, 1, 2, 5, r.getrandbits(32) - (1 << 31)]))
    entry = bytearray(r.bytes(0x20))
    put(entry, 0, 'II', SURF, ITEMS if n else 0)
    gen = r.getrandbits(32) & 0x7fffffff
    put(entry, 0x14, 'i', gen - r.choice([0, 1, 15, 16, 17, 20, -3, r.randrange(30)]))
    set_return(log, 3, r.choice([0, 16, 64, 0x4000, r.getrandbits(32)]))
    regions = {ENTRY: entry, SURF: struct.pack('<I', VT) + r.bytes(0xc), VT: vtable({}, 8), VB: struct.pack('<ii', 0, off), ITEMS: items, LOG: log,
               0x8bcc40: struct.pack('<I', gen), 0x8bd524: struct.pack('<I', r.choice([1, 1, 0, 4]))}
    return Call(0x591010, [ENTRY, r.getrandbits(32)], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
