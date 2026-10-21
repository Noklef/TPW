# The deque-of-dwords iterator (src/game/core/dequeiter: 0x632200, 0x632230, 0x632290, 0x632310)
# and the object constructor 0x632380 (and the deque of 8-byte elements emptied by 0x47f340) against the originals: five blocks of 0x1000 bytes behind a
# map, iterators anywhere in them, steps that stay inside, cross one block or several, either way. The command queue's post (0x47f410): the global deque
# (0x7b4ba8) empty, or its last block with room, full, or full at the map's end (the map grown), its critical section's calls recorded. Its
# show-next (0x47f7d0): the first element's text (a record table of 0-4 text records, some not terminated, the index about them) to a
# recorder's slot 9, the cursor anywhere in its block or at its last element (the map's blocks out of order), the count 0-3 or more.
# usage: dequeiter.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import IMPORT_TABLE, LOG, Bench, Call, HEAP, RAM, heap, import_table, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x632200, 0x632222), (0x632230, 0x632290), (0x632290, 0x6322dc), (0x632310, 0x632343), (0x632380, 0x6323bd), (0x47f340, 0x47f420), (0x67acb0, 0x67acb1), (0x47f410, 0x47f61e), (0x5c8bb0, 0x5c8bd0), (0x6322e0, 0x632310), (0x47f7d0, 0x47f903), (0x6ace60, 0x6acea0), (0x485ac0, 0x485af4)])
CALLS = ['Set', 'Plus', 'Diff', 'Prev', 'Ctor', 'Clear', 'Post', 'ShowNext']
IMPORTS = {'EnterCriticalSection': (1, 0x6fd140, 1, 0, 0), 'LeaveCriticalSection': (2, 0x6fd130, 1, 0, 0)}
QUEUE = 0x7b4ba8
IT, IT2, OUT, MAP, BLOCKS = RAM, RAM + 0x20, RAM + 0x40, RAM + 0x100, RAM + 0x2000


def iterator(block, off):
    first = BLOCKS + 0x1000 * block
    return struct.pack('<IIII', first, first + 0x1000, first + 4 * off, MAP + 4 * block)


TABLE, RECORDS, TARGET = RAM + 0x8000, RAM + 0x8100, RAM + 0x9000


def show_next(r):
    """0x47f7d0 on the queue: the begin iterator in one of the five blocks, its element (the table, an index) at the cursor."""
    log = log_region()
    table = import_table(log, IMPORTS)
    set_return(log, 1, r.getrandbits(32))
    set_return(log, 2, r.getrandbits(32))
    blocks = bytearray(r.bytes(0x5000))
    a = r.randrange(5)
    order = r.sample(range(5), 5)   # (the map's blocks out of order: a block's end is not the next one's start)
    k = r.choice([0, 1, 510, 511, 511, r.randrange(512)])
    count = r.choice([0, 1, 1, 2, 3, 600, r.getrandbits(32)])
    n = r.randrange(0, 5)
    put(blocks, 0x1000 * order[a] + 8 * k, 'II', TABLE, r.choice([0, 1, 2, 3, 4, n, n - 1, 0xffffffff]) & 0xffffffff)
    q = bytearray(r.bytes(0x48))
    first = BLOCKS + 0x1000 * order[a]
    put(q, 4, 'IIII', first, first + 0x1000, first + 8 * k, MAP + 4 * a)
    put(q, 0x24, 'III', MAP, 5, count)
    tab = bytearray(r.bytes(0x20))
    put(tab, 8, 'II', n, RECORDS)
    pool = bytearray()
    for i in range(5):
        body = bytes(r.randrange(1, 256) for _ in range(r.choice([0, 1, 3, 8, 12]))) + bytes([0]) * r.choice([1, 1, 2])
        head = r.choice([1] * 6 + [2, 0x101]) | (len(body) << 8 if r.random() < 0.9 else r.randrange(0x40) << 8)
        rec = bytearray(struct.pack('<I', head) + body + bytes(0x40 - 4 - len(body)))
        if r.random() < 0.15:
            rec[4 + ((len(body) + 3) & ~3)] = 1   # (not terminated)
        pool += rec
    target = bytearray(struct.pack('<I', TARGET + 0x10)) + bytearray(0xc) + vtable({9: 1})
    regions = {QUEUE: q, LOG: log, IMPORT_TABLE: table, HEAP: heap(), MAP: struct.pack('<5I', *[BLOCKS + 0x1000 * i for i in order]),
               BLOCKS: blocks, TABLE: tab, RECORDS: struct.pack('<5I', *[RECORDS + 0x100 + 0x40 * i for i in range(5)]) + bytes(0xec) + bytes(pool),
               TARGET: target, 0x7b4ba0: struct.pack('<I', TARGET), 0x78ba80: bytes(4), 0x7ba058: bytes(0x10)}
    return Call(0x47f7d0, [], regions, conv='c', ret='a')


def case(r):
    op = r.deal(OPS)
    if op == 'ShowNext':
        return show_next(r)
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
    if op == 'Post':   # the queue: +4 the begin iterator, +0x14 the end one {first, end, cursor, map slot}, +0x24 the map, +0x28 its size, +0x2c the count
        log = log_region()
        table = import_table(log, IMPORTS)
        set_return(log, 1, r.getrandbits(32))
        set_return(log, 2, r.getrandbits(32))
        q = bytearray(r.bytes(0x48))
        size = r.choice([2, 3, 5])
        a = r.randrange(size)
        b = r.choice([a, size - 1, r.randrange(a, size)])
        cursor = r.choice([0, 1, 2, 511, 512, 512, 512, r.randrange(1, 513)])
        put(q, 4, 'IIII', BLOCKS + 0x1000 * a, BLOCKS + 0x1000 * a + 0x1000, BLOCKS + 0x1000 * a + 8 * r.randrange(512), MAP + 4 * a)
        put(q, 0x14, 'IIII', BLOCKS + 0x1000 * b, BLOCKS + 0x1000 * b + 0x1000, BLOCKS + 0x1000 * b + 8 * cursor, MAP + 4 * b)
        put(q, 0x24, 'III', MAP, size, r.choice([0, 0, 1, 2, 3, 700, r.getrandbits(32)]))
        regions.update({QUEUE: q, LOG: log, IMPORT_TABLE: table, HEAP: heap(fail=r.choice([0] * 12 + [1, 2])), 0x78ba88: r.bytes(4)})
        return Call(0x47f410, [r.getrandbits(32), r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='c', ret='a')
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
