# The object controls' table of records (src/game/world/objectdefs: 0x4d3e00) against the original:
# up to 12 definitions (ids repeated, so some are found in the table and some through the cache), tables
# that start empty or with records, and a cache that names a record, names none, or is stale.
# usage: objectcontrols.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x4d3e00, 0x4d3ec9), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f), (0x4d3d10, 0x4d3d80), (0x4d3d80, 0x4d3df0), (0x5da3c0, 0x5da3c1), (0x4d3de0, 0x4d3e00), (0x4ca8c0, 0x4ca8e0), (0x4e2740, 0x4e2770)])
GAME = 0x1d943c  # the table's offset in the game object [0x80239c] points to
DEFS, ITEMS = 0x7890a0, RAM + 0x4000


def call_count(table):
    return max(1, int.from_bytes(table[0x12c0:0x12c4], 'little', signed=True))


def find_case(r):
    table = bytearray(r.bytes(0x1400))
    have = r.choice([0, 1, 3, 6, 12])
    put(table, 0x12c0, 'i', r.choice([have, have, -1, 0]))
    for k in range(have):
        put(table, 0x20 * k, 'H', r.choice([1, 2, 3, 4, 5, 9]))
    put(table, 0x12c4, 'H', r.choice([0, 1, 2, 3, 9]))
    put(table, 0x12c8, 'I', r.choice([0, RAM + 0x20 * r.randrange(8)]))
    return Call(r.choice([0x4d3d10, 0x4d3d80]), [RAM, r.choice([0, 1, 2, 3, 4, 5, 9, 0x10001, 0xffff0001, r.getrandbits(32)]), r.choice([0, 1])], {RAM: table},
                conv='t', ret='a')


def case(r):
    x = r.random()
    if x < 0.15:  # the getters over the game's table: the table at RAM, the game object right below it
        call = find_case(r)
        regions = dict(call.regions)
        regions[0x80239c] = put(bytearray(4), 0, 'I', RAM - GAME)
        if r.random() < 0.5:
            put(regions[RAM], 0x12c8, 'I', RAM + 0x20 * r.randrange(max(1, min(6, call_count(regions[RAM])))))
            return Call(0x4ca8c0, [r.choice([0, 1, 2, 3, 4, 5, 9, 0x10001, r.getrandbits(32)])], regions, conv='s', ret='a')
        regions[RAM + 0x2000] = put(bytearray(r.bytes(0x20)), 0xe, 'H', r.choice([0, 1, 2, 3, 4, 5, 9, r.getrandbits(16)]))
        return Call(0x4e2740, [RAM + 0x2000], regions, conv='t', ret='a')
    if x < 0.6:
        return find_case(r)
    n = r.choice([0, 1, 2, 5, 12])
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(12 * 0x200))
    put(defs, 0x528, 'i', n)
    put(defs, 0x72c, 'i', r.choice([-1, -1, 0, 3]))  # the walk cursor (First sets it)
    for i in range(12):
        put(defs, i * 4, 'I', ITEMS + 0x200 * i)
        put(items, 0x200 * i, 'H', r.choice([1, 2, 3, 4, 5, r.getrandbits(16)]))
        if r.random() < 0.4:
            put(items, 0x200 * i + 0x180, 'I', 0)
    table = bytearray(r.bytes(0x1400))
    have = r.choice([0, 0, 1, 3, 6])
    put(table, 0x12c0, 'i', have)
    for k in range(have):
        put(table, 0x20 * k, 'H', r.choice([1, 2, 3, 4, 5, 9]))
    cache = r.choice([0, 0, 1, 2, 3, 9, r.getrandbits(16)])
    put(table, 0x12c4, 'H', cache)
    put(table, 0x12c8, 'I', r.choice([0, RAM + 0x20 * r.randrange(8), RAM + 0x20 * r.randrange(8)]))
    return Call(r.choice([0x4d3e00, 0x4d3de0]), [RAM], {RAM: table, DEFS: defs, ITEMS: items}, conv='t')


if __name__ == '__main__':
    B.main(case)
