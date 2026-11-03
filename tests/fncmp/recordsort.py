# The records' quicksort (src/game/core/recordsort: 0x57ced0, 0x57d090) against the originals: up to 40 records of 1-20 bytes (the key any dword inside, over the record's end now and then for short
# strides), keys from a small set or any, the range all, part, one record or the wrong way round.
# usage: recordsort.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

B = Bench([(0x57ced0, 0x57cff4), (0x57d090, 0x57d0bb)])
SORT, TEMP, ARRAY = RAM, RAM + 0x40, RAM + 0x100
TOP = 0xc0000000


def case(r):
    stride = r.choice([4, 8, 12, 16, 20, 5, 7, 13, 1, 2, 3])
    key_at = r.randrange(0, max(stride - 3, 1))
    n = r.choice([1, 2, 3, r.randrange(1, 12), r.randrange(1, 41)])
    a = bytearray(r.bytes(stride * n + 8))
    small = [r.getrandbits(32) for _ in range(4)]
    choice = r.random() < 0.5
    for i in range(n):
        struct.pack_into('<I', a, i * stride + key_at, r.choice(small) if choice else r.getrandbits(32))
    first, last = r.choice([(0, n - 1), (0, n - 1), (r.randrange(n), n - 1), (0, r.randrange(n)), (n - 1, 0), (0, 0)])
    regions = {TEMP: r.bytes(0x20), ARRAY: a}
    if r.deal([0, 1]):
        regions[SORT] = struct.pack('<3I', stride, key_at, TEMP)
        return Call(0x57ced0, [SORT, ARRAY, first, last], regions, conv='t', ret='v')
    regions[SORT] = r.bytes(0xc)
    return Call(0x57d090, [SORT, ARRAY, TEMP, stride, key_at, first, last], regions, conv='t', ret='v')


if __name__ == '__main__':
    B.main(case)
