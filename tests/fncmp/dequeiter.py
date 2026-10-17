# The deque-of-dwords iterator (src/game/core/dequeiter: 0x632200, 0x632230, 0x632290, 0x632310)
# and the object constructor 0x632380 (and the deque of 8-byte elements emptied by 0x47f340) against the originals: five blocks of 0x1000 bytes behind a
# map, iterators anywhere in them, steps that stay inside, cross one block or several, either way.
# usage: dequeiter.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

B = Bench([(0x632200, 0x632222), (0x632230, 0x632290), (0x632290, 0x6322dc), (0x632310, 0x632343), (0x632380, 0x6323bd), (0x47f340, 0x47f420), (0x67acb0, 0x67acb1)])
CALLS = ['Set', 'Plus', 'Diff', 'Prev', 'Ctor', 'Clear']
IT, IT2, OUT, MAP, BLOCKS = RAM, RAM + 0x20, RAM + 0x40, RAM + 0x100, RAM + 0x2000


def iterator(block, off):
    first = BLOCKS + 0x1000 * block
    return struct.pack('<IIII', first, first + 0x1000, first + 4 * off, MAP + 4 * block)


def case(r):
    op = r.choice(CALLS)
    kb, off = r.randrange(5), r.choice([0, 1, 1023, r.randrange(1024)])
    regions = {RAM: bytearray(r.bytes(0x90)), MAP: struct.pack('<5I', *[BLOCKS + 0x1000 * i for i in range(5)]),
               BLOCKS: r.bytes(0x5000), OUT: r.bytes(0x10)}
    regions[RAM][0:16] = iterator(kb, off)
    if op == 'Clear':   # +4 the begin iterator, +0x14 the end one, +0x24 the map, +0x2c the count
        first = BLOCKS + 0x1000 * kb
        cursor = first + 0x1000 - 8 * r.choice([1, 2, 3, 5, 600, 513])
        obj = bytearray(r.bytes(0x40))
        put(obj, 4, 'IIII', first, first + 0x1000, cursor, MAP + 4 * kb)
        put(obj, 0x2c, 'I', r.choice([0, 1, 2, 3, 4, 6, 8, 600]))
        regions[RAM] = obj
        regions[HEAP] = heap()
        return Call(0x47f340, [RAM], regions, conv='t', ret='v')
    if op == 'Set':
        return Call(0x632200, [IT, BLOCKS + 0x1000 * kb + 4 * off, MAP + 4 * r.randrange(5)], regions, conv='t', ret='a')
    if op == 'Plus':
        lo, hi = -(kb * 1024 + off), (4 - kb) * 1024 + 1023 - off
        edges = [m for m in (-off - 1024, -off - 1025, -off - 1, -off, 1023 - off, 1024 - off, 1025 - off, -2048 - off) if lo <= m <= hi]
        n = r.choice([0, 1, -1, r.randrange(lo, hi + 1), r.randrange(max(lo, -2000), min(hi, 2000) + 1)] + edges * 3)
        return Call(0x632230, [IT, OUT, n & 0xffffffff], regions, conv='t', ret='a')
    if op == 'Diff':
        regions[RAM] = regions[RAM] + iterator(r.choice([kb, kb, r.randrange(5)]), r.randrange(1024))
        return Call(0x632290, [IT, IT2], regions, conv='t', ret='a')
    if op == 'Prev':
        if off and r.random() < 0.5:
            regions[RAM][0:16] = iterator(max(kb, 1), 0)
        else:
            regions[RAM][0:16] = iterator(max(kb, 1), off)
        return Call(0x632310, [IT], regions, conv='t', ret='a')
    return Call(0x632380, [IT + r.choice([0, 0x40])], regions, conv='t', ret='a')


OPS = CALLS


if __name__ == '__main__':
    B.main(case)
