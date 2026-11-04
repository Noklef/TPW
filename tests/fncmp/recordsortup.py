# The other quicksorts of src/game/core/recordsort against the originals: records the smallest first (0x57cda0, 0x57d060), pointers to records by a dword key (0x57cbc0, 0x57d000) and by a float key, NaNs too (0x57ccb0, 0x57d030):
# keys from a small set or any, the range all, part, one record or the wrong way round. The temp is mapped.
# usage: recordsortup.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, fb  # noqa: E402

B = Bench([(0x57cbc0, 0x57cca9), (0x57ccb0, 0x57cd8f), (0x57cda0, 0x57cec2), (0x57d000, 0x57d02b), (0x57d030, 0x57d05b), (0x57d060, 0x57d08b)])
SORT, TEMP, ARRAY, RECORDS = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x400


def bounds(r, n):
    return r.choice([(0, n - 1), (0, n - 1), (0, n - 1), (r.randrange(n), n - 1), (0, r.randrange(n)), (n - 1, 0), (0, 0)])


def records(r):
    stride = r.choice([4, 8, 12, 16, 20, 5, 7, 13, 1, 2, 3])
    key_at = r.randrange(0, max(stride - 3, 1))
    n = r.choice([1, 2, 3, r.randrange(1, 12), r.randrange(1, 41)])
    a = bytearray(r.bytes(stride * n + 8))
    small = [r.getrandbits(32) for _ in range(4)]
    choice = r.random() < 0.5
    for i in range(n):
        struct.pack_into('<I', a, i * stride + key_at, r.choice(small) if choice else r.getrandbits(32))
    first, last = bounds(r, n)
    regions = {TEMP: r.bytes(0x20), ARRAY: a}
    if r.deal([0, 1]):
        regions[SORT] = struct.pack('<3I', stride, key_at, TEMP)
        return Call(0x57cda0, [SORT, ARRAY, first, last], regions, conv='t', ret='v')
    regions[SORT] = r.bytes(0xc)
    return Call(0x57d060, [SORT, ARRAY, TEMP, stride, key_at, first, last], regions, conv='t', ret='v')


def pointers(r, by_float):
    n = r.choice([1, 2, 3, r.randrange(1, 12), r.randrange(1, 41)])
    stride = 0x10
    key_at = r.randrange(0, 13)
    data = bytearray(r.bytes(stride * n))
    if by_float:
        small = [fb(x) for x in (0.0, 1.0, -1.0, 0.5)] + [0x7fc00000, 0x7fa00000]
        draw = lambda: r.choice(small) if r.random() < 0.6 else fb(r.uniform(-4, 4))
    else:
        small = [r.getrandbits(32) for _ in range(4)]
        draw = lambda: r.choice(small) if r.random() < 0.5 else r.getrandbits(32)
    order = list(range(n))
    r.shuffle(order)
    for i in range(n):
        struct.pack_into('<I', data, i * stride + key_at, draw())
    array = struct.pack('<%dI' % n, *[RECORDS + stride * i for i in order])
    first, last = bounds(r, n)
    regions = {ARRAY: array, RECORDS: data}
    if r.deal([0, 1]):
        regions[SORT] = struct.pack('<3I', r.getrandbits(32), key_at, r.getrandbits(32))
        return Call(0x57ccb0 if by_float else 0x57cbc0, [SORT, ARRAY, first, last], regions, conv='t', ret='v')
    regions[SORT] = r.bytes(0xc)
    return Call(0x57d030 if by_float else 0x57d000, [SORT, ARRAY, TEMP, stride, key_at, first, last], regions, conv='t', ret='v')


def case(r):
    op = r.deal(['records', 'up', 'float'])
    return records(r) if op == 'records' else pointers(r, op == 'float')


if __name__ == '__main__':
    B.main(case)
