# The texture pages' square allocator (src/game/gfx/texatlas: 0x58c3c0) against the original: sizes 1-0x100 and past it (0, negative
# too), 0-8 free rectangles (and 31, 32: the 33rd overlaps the page, the count and the pages made) of widths and heights around the size,
# exactly it, smaller or a whole page, the page tracked blocks (TrackedGlobal_Alloc: GlobalAlloc served by heap(), failing now and then).
# usage: texatlas.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Alloc)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, global_heap, heap  # noqa: E402

B = Bench([(0x58c3c0, 0x58c605), (0x579b40, 0x579c13)])
CALLS = {'Alloc': 0x58c3c0}
FREE, COUNT, TABLE = 0x8c4928, 0x8bb214, 0x87b1f8   # (the free rectangles to 0x8c4b28, then the page, the pages made, the count)


def case(r):
    r.deal(OPS)
    size = r.choice([1, 2, 4, 8, 16, 32, 64, 128, 0xff, 0x100, 0x101, 0x300, 0, 0xffffffff, r.randrange(1, 0x100)])
    s = size - (1 << 32) if size >> 31 else size
    fail = r.random() < 0.15
    regions = {HEAP: heap(size=0x100 if fail else 0x40100, fail=1 if fail else 0), **global_heap()}
    regions[COUNT] = struct.pack('<I', r.choice([0, 1, 5])) + r.bytes(4)
    regions[TABLE] = r.bytes(0x40)
    count = r.choice([0, 1, 2, 2, 3, 4, 5, 8, 31, 32, 0xffffffff])
    free = bytearray(0x210)
    for k in range(32):
        side = lambda: r.choice([s, s, s, s + 1, s - 1, 0x100, 0x100 - s, r.randrange(0, 0x101), r.getrandbits(32)]) & 0xffffffff
        struct.pack_into('<4I', free, 16 * k, r.randrange(0, 0x100), r.randrange(0, 0x100), side(), side())
    struct.pack_into('<3I', free, 0x200, r.choice([HEAP + 0x1000, 0x10000000, 0]), r.getrandbits(8), count)
    regions[FREE] = free
    regions[RAM] = r.bytes(8)
    return Call(0x58c3c0, [RAM, size], regions, conv='c', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
