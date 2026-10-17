# The tracked global blocks of src/game/core/gameheap (0x579b40 alloc, 0x579b80 free) against the originals: the count at 0x8bb214 small,
# at the 0x8000 limit or past it (and 0, -1, 0x80000000 for the alloc), the pairs at 0x87b1f8 holding the block asked for (as it is or 4
# below), another or none, GlobalAlloc failing now and then (the heap region serves GlobalAlloc / GlobalFree).
# usage: trackedglobal.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alloc,Free)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, global_heap, heap  # noqa: E402

CALLS = {'Alloc': 0x579b40, 'Free': 0x579b80}
B = Bench([(0x579b40, 0x579c13)])
COUNT, TABLE, LOW = 0x8bb214, 0x87b1f8, 0x87b1e0   # (the pairs from index -3, for the counts of -1 and below)
BLOCKS = [HEAP + 0x40, HEAP + 0x44, HEAP + 0x80, HEAP + 0x84, HEAP + 0x100, 0, 4]


def case(r):
    name = r.choice(OPS)
    regions = {HEAP: heap(fail=r.choice([0] * 7 + [1])), **global_heap()}
    big = r.random() < 0.1
    if big:
        count = r.choice([0x7fff, 0x8000, 0x8001] if name == 'Free' else [0x7fff, 0x8000, 0x8001, 0x80000000])
        table = bytearray(0x18 + 0x8002 * 8)
        for _ in range(r.randrange(0, 6)):   # a few pairs anywhere
            k = r.randrange(0, 0x8002)
            struct.pack_into('<2I', table, 0x18 + 8 * k, r.choice(BLOCKS), r.choice(BLOCKS))
    else:
        count = r.choice([0, 1, 2, 3, 4, 5, 6, 0xffffffff, 0xfffffffe] + ([0x80000000] if name == 'Alloc' else []))
        table = bytearray(0x18 + 8 * 8)
        for k in range(len(table) // 4):
            struct.pack_into('<I', table, 4 * k, r.choice(BLOCKS + [r.getrandbits(32)]))
    regions[LOW] = table
    regions[COUNT] = struct.pack('<I', count) + r.bytes(4)
    arg = r.choice([0, 1, 0x10, 0x100]) if name == 'Alloc' else r.choice(BLOCKS + [r.getrandbits(32)])
    return Call(CALLS[name], [arg], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
