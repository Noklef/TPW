# The texture cache's sorts (src/game/gfx/texcache: 0x58cba0, 0x58cc30, 0x58e1a0, 0x58e790, 0x58e820) against the originals: arrays of entries' or items' pointers with keys from a small set or any, chains of 0-40 items, now and then
# the whole 0x1000 and a few past them. The real sorts of src/game/core/recordsort run at 0x8bcb98 (its +8 a stack address out of the comparison).
# usage: texcachesort.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x58cba0, 0x58cc2a), (0x58cc30, 0x58ccba), (0x58e1a0, 0x58e1c9), (0x58e790, 0x58e81b), (0x58e820, 0x58e8c4), (0x57cbc0, 0x57cca9), (0x57cda0, 0x57cec2), (0x57d000, 0x57d02b), (0x57d060, 0x57d08b), (0x67b110, 0x67b140)])
CACHE, LIST, ARRAY, DATA = RAM, RAM + 0x40, RAM + 0x100, RAM + 0x1000
SORT = 0x8bcb98
SORT_REGION = {SORT: bytes(0xc)}
IGNORE = [(SORT + 8, 4)]


def keys(r, n):
    small = [r.getrandbits(32) for _ in range(4)]
    choice = r.random() < 0.5
    return [r.choice(small) if choice else r.getrandbits(32) for _ in range(n)]


def entries(r):
    address = r.deal([0x58cba0, 0x58cc30])
    n = r.choice([1, 2, 3, r.randrange(1, 12), r.randrange(1, 60)])
    data = bytearray(r.bytes(0x20 * n))
    for i, k in enumerate(keys(r, n)):
        put(data, 0x20 * i + (0xc if address == 0x58cba0 else 0x10), 'I', k)
    order = list(range(n))
    r.shuffle(order)
    first, last = r.choice([(0, n - 1), (0, n - 1), (r.randrange(n), n - 1), (0, r.randrange(n)), (0, 0)])
    return Call(address, [CACHE, ARRAY, first, last], {CACHE: r.bytes(4), ARRAY: struct.pack('<%dI' % n, *[DATA + 0x20 * i for i in order]), DATA: data}, conv='t', ret='v')


def items_by_key(r):
    n = r.choice([1, 2, 3, r.randrange(1, 12), r.randrange(1, 40)])
    data = bytearray(r.bytes(0x60 * n))
    for i, k in enumerate(keys(r, n)):
        put(data, 0x60 * i + 0x38, 'I', k)
    order = list(range(n))
    r.shuffle(order)
    first, last = r.choice([(0, n - 1), (0, n - 1), (r.randrange(n), n - 1), (0, 0), (n - 1, 0)])
    return Call(0x58e1a0, [CACHE, ARRAY, first, last], {**SORT_REGION, CACHE: r.bytes(4), ARRAY: struct.pack('<%dI' % n, *[DATA + 0x60 * i for i in order]), DATA: data}, conv='t', ret='v', ignore=IGNORE)


def chain(r, stride):
    n = 0x1000 + r.choice([0, 1, 3, 9]) if r.randrange(60) == 0 else r.choice([0, 1, 2, 3, r.randrange(1, 12), r.randrange(1, 41)])
    data = bytearray(r.bytes(stride * n))
    order = list(range(n))
    r.shuffle(order)
    for i, k in enumerate(keys(r, n)):
        put(data, stride * order[i] + 4, 'I', DATA + stride * order[i + 1] if i + 1 < n else 0)
        put(data, stride * i + 0x38, 'I', k)
    return struct.pack('<I', DATA + stride * order[0] if n else 0), data


def by_key(r):
    head, data = chain(r, 0x40)
    regions = {**SORT_REGION, CACHE: r.bytes(4), LIST: head}
    if data:
        regions[DATA] = data
    return Call(0x58e790, [CACHE, LIST], regions, conv='t', ret='v', ignore=IGNORE, steps=40_000_000)


def by_address(r):
    head, data = chain(r, 0x40)
    regions = {**SORT_REGION, CACHE: r.bytes(4), LIST: head}
    if data:
        regions[DATA] = data
    return Call(0x58e820, [CACHE, LIST], regions, conv='t', ret='v', ignore=IGNORE, steps=40_000_000)


def case(r):
    return r.deal([entries, items_by_key, by_key, by_address])(r)


if __name__ == '__main__':
    B.main(case)
