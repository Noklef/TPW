# The object definition lookups, walk and schema slots (src/game/world/objectdefs: 0x412e90,
# 0x412ef0, 0x412f50, 0x412e60, 0x412e70, 0x412c90, 0x4143f0, 0x413410) against the original: ids among the list
# (repeated, missing), the cache hit (its pointer anything, null included) or missed, counts zero,
# negative or up to the full 0x14a; the walk cursor about the end. The marked cells (0x413410): grids of up to 22 columns
# and 4 rows (or none, or negative counts) with cells of kind 9 and 0xa or not (IntPair_Copy 0x4d99c0 run).
# usage: objectdefs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,At390,At410,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x412e90, 0x412eea), (0x412ef0, 0x412f4c), (0x412f50, 0x412fac), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f),
           (0x412c90, 0x412ca4), (0x4143f0, 0x414404), (0x413410, 0x4135ee), (0x4d99c0, 0x4d99d3)])
DEFS, ITEMS, DEF, GRID, REF = RAM, RAM + 0x1000, RAM + 0x3000, RAM + 0x3800, RAM + 0x3c00
CALLS = {'Find': 0x412e90, 'At390': 0x412ef0, 'At410': 0x412f50, 'First': 0x412e60, 'Next': 0x412e70, 'Schema': 0x412c90,
         'TrackSchema': 0x4143f0, 'Cells': 0x413410}


def cells(r):  # a definition (through a variable) and its cell grid (+0x18): the column and row counts, then cells of 8 bytes, 20 to a row
    obj = bytearray(r.bytes(0x500))
    grid = bytearray(r.bytes(0x400))
    put(grid, 0, 'ii', r.choice([0, 1, 2, 3, 5, 20, 22, r.randrange(-2, 1)]), r.choice([0, 1, 2, 4, r.randrange(-1, 5)]))
    for k in range(8, 0x3f8, 8):
        put(grid, k, 'I', r.choice([9, 10, 9, 10, 8, 11, 0x109, r.getrandbits(32)]) if r.random() < 0.15 else r.choice([0, 1, 2, 3]))
    put(obj, 0x18, 'I', GRID)
    return Call(0x413410, [r.getrandbits(32), REF], {DEF: obj, GRID: grid, REF: struct.pack('<I', DEF)}, conv='t', ret='v')


def case(r):
    op = r.choice(OPS)
    if op == 'Cells':
        return cells(r)
    n = r.choice([0, 1, r.randrange(2, 20), r.randrange(20, 0x14b), r.randrange(-2, 0)])
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(0x14a * 0x10))
    for i in range(0x14a):
        put(defs, i * 4, 'I', ITEMS + 0x10 * r.randrange(0x14a))
        put(items, i * 0x10, 'H', r.choice([i, r.randrange(16), r.getrandbits(16)]))
    if op in ('Schema', 'TrackSchema'):
        return Call(CALLS[op], [r.getrandbits(32), r.choice([0, 1, r.randrange(64), r.getrandbits(32)])], {}, conv='t', ret='a')
    if op == 'First':
        put(defs, 0x528, 'i', n)
        return Call(0x412e60, [DEFS], {DEFS: defs, ITEMS: items}, conv='t', ret='a')
    if op == 'Next':  # the cursor about the end, past it, negative
        put(defs, 0x528, 'i', n)
        put(defs, 0x72c, 'i', max(-1, r.choice([-1, 0, n - 2, n - 1, n, r.randrange(-1, 0x14a)])))
        return Call(0x412e70, [DEFS], {DEFS: defs, ITEMS: items}, conv='t', ret='a')
    id_ = r.choice([r.randrange(16), r.randrange(0x14a), r.getrandbits(16)])
    put(defs, 0x528, 'i', n)
    put(defs, 0x730, 'HxxI', r.choice([id_, id_ ^ 1, r.getrandbits(16)]), r.choice([0, ITEMS, r.getrandbits(32)]))
    return Call(CALLS[op], [DEFS, id_ | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)],
                {DEFS: defs, ITEMS: items}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
