# The ride list accessors (src/game/world/ridelist) against the originals: the system flag set or not, an empty list or up to four records
# (ids, link ids, arrays of up to four dwords), records and indexes in and outside the list and the arrays.
# usage: ridelist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Find,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# name: (address, size, arguments: id, rec, rec+index, rec+index+value, recword, linked)
CALLS = {'Find': (0x55a070, 0x40, 'id'), 'Set': (0x55a0b0, 0x50, 'rec2v'), 'Get': (0x55a390, 0x50, 'rec2'), 'SetW': (0x55a300, 0x30, 'recv'),
         'GetW': (0x55a330, 0x30, 'rec'), 'G9c': (0x55a360, 0x30, 'rec'), 'Link': (0x55a3e0, 0x90, 'id2')}
B = Bench([(a, a + n) for (a, n, _s) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])
NODES, ARRAYS = RAM + 0x1000, RAM + 0x4000


def case(r):
    name = r.choice(OPS)
    address, _n, shape = CALLS[name]
    count = r.choice([0, 1, 2, 3, 4])
    ids = r.sample(range(1, 9), count)
    nodes = bytearray(r.bytes(0x100 * 4))
    arrays = bytearray(r.bytes(0x40 * 4))
    for i, rid in enumerate(ids):
        o = 0x100 * i
        put(nodes, o, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, o + 8, 'I', rid)
        put(nodes, o + 0x14, 'I', r.choice(ids + [99]))
        put(nodes, o + 0x1c, 'I', ARRAYS + 0x40 * i)
        put(nodes, o + 0x8c, 'i', r.choice([0, 1, 2, 4, -1]))
    regions = {NODES: nodes, ARRAYS: arrays, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1])),
               0x8791b0: struct.pack('<I', NODES if count else 0)}
    rec = r.choice([0] + [NODES + 0x100 * i for i in range(count)]) if count else 0
    ident = r.choice(ids + [0, 99, 5]) if ids else r.choice([0, 1, 5])
    index = r.choice([0, 1, 2, 3, 4, 5, -1, 0x7fffffff, 0x80000000])
    value = r.choice([0, 1, 0x7fff, 0x8000, 0xffff, 0x12345678, 0xffffffff])
    args = {'id': [ident], 'rec2v': [rec, index, value], 'rec2': [rec, index], 'recv': [rec, value], 'rec': [rec], 'id2': [ident, index]}[shape]
    return Call(address, args, regions, conv='c', ret='v' if name in ('Set', 'SetW') else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
