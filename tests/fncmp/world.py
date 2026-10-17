# The world's small methods (src/game/world/world: 0x51aad0, 0x51aaf0, 0x51ab20, 0x51ab60,
# 0x51ad30, 0x51ae10, 0x51a9d0) against the original: arrival vehicles zero, equal or not; the visitor
# count about its wrap; ride numbers 0 to 256 (the bitsets' range) on found definitions, the
# walk's count zero to full with a null pointer at times, the left-out flag either way, and
# wanted bitsets inside or outside what the database has; the bus stop's four cells (0x51a9d0) with handles in
# the cells' slots, things of the right kind or not.
# usage: world.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Vehicle,Visitor,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x51aad0, 0x51aaee), (0x51aaf0, 0x51ab1a), (0x51ab20, 0x51ab56), (0x51ab60, 0x51ac18),
           (0x51ad30, 0x51ad33), (0x51ae10, 0x51ae24), (0x51a9d0, 0x51aae0), (0x4d8650, 0x4d8690), (0x412e60, 0x412e6d), (0x412e70, 0x412e8f),
           (0x412e90, 0x412eea), (0x5da3c0, 0x5da3c1)])
WORLD, WANTED, ITEMS = RAM - 0x1da700, RAM + 0x100, RAM + 0x1000
DEFS, BUILT, SCRATCH, NITEMS = 0x7890a0, 0x802080, 0x7cfb70, 24
OPS_ALL = ['Vehicle', 'Visitor', 'Mark', 'Has', 'Empty', 'Record', 'BusStop']


def struct_pack_h(v):
    return bytes([v & 0xff, v >> 8])


def struct_pack_i(v):
    return bytes([(v >> (8 * i)) & 0xff for i in range(4)])


def case(r):
    op = r.choice(OPS)
    world = bytearray(r.bytes(0x40))
    if op == 'Vehicle':
        a = r.choice([0, r.randrange(1, 4), r.getrandbits(16)])
        put(world, 0x2a, 'HH', a, r.choice([a, 0, r.randrange(1, 4), r.getrandbits(16)]))
        return Call(0x51aad0, [WORLD], {RAM: world}, conv='t', ret='a')
    if op == 'Visitor':
        put(world, 0x14, 'I', r.choice([0, 1, 0x7fffffff, 0xffffffff, r.getrandbits(32)]))
        return Call(0x51aaf0, [WORLD], {RAM: world}, conv='t', ret='a')
    if op in ('Empty', 'Record'):
        return Call(0x51ad30 if op == 'Empty' else 0x51ae10, [RAM + 0x10], {RAM: world}, conv='t', ret='a')
    if op == 'BusStop':   # the position globals, a handle (or not) in each of the four cells' slots, the things behind the handles
        y, x = r.choice([2, 3, 4]), r.randrange(0, 30)
        v = (y << 7) + x + 1
        regions = {0x7855ac: put(bytearray(8), 0, 'II', x, y)}
        cells = [v, v + 1, (v - 0x100) & 0xffff, (v - 0xff) & 0xffff]
        table = bytearray(r.bytes(0x70))
        for i, cell in enumerate(cells):
            handle = r.choice([0, 1, 2, 3, 4, 5])
            regions[WORLD + 0x2b8 + cell * 0x44] = struct_pack_h(handle)
        for h in range(1, 6):
            thing = RAM + 0x4000 + 0x400 * h
            put(table, 20 * h, 'I', thing)
            regions[thing + 2] = bytes([r.choice([1, 1, 1, 0, 2])])
            regions[thing + 0x220] = struct_pack_i(r.choice([0x15, 0x15, 0x14, 0, r.getrandbits(32)]))
        regions[0x7cfb90] = table
        return Call(0x51a9d0, [WORLD], regions, conv='t', ret='a')
    defs = bytearray(r.bytes(0x740))
    items = bytearray(r.bytes(NITEMS * 0xa0))
    count = r.choice([0, 1, r.randrange(1, NITEMS + 1), NITEMS])
    for i in range(NITEMS):
        put(defs, i * 4, 'I', ITEMS + 0xa0 * i)
        put(items, i * 0xa0, 'H', i + 1)
        put(items, i * 0xa0 + 0x94, 'II', r.choice([0, 1, 8, 9, 256, r.randrange(1, 257), r.randrange(1, 17)]),
            r.choice([0, 0, 1, r.getrandbits(32)]))
    put(defs, 0x528, 'i', count)
    regions = {DEFS: defs, ITEMS: items, BUILT: r.bytes(0x20), SCRATCH: r.bytes(0x20)}
    if op == 'Mark':
        put(defs, 0x730, 'HxxI', 0, 0)  # no cache hit
        count = max(count, 1)  # the definition found, as the original needs
        put(defs, 0x528, 'i', count)
        id_ = r.randrange(1, count + 1)
        return Call(0x51ab20, [r.getrandbits(32), id_ | (r.getrandbits(16) << 16 if r.random() < 0.3 else 0)],
                    regions, conv='t', ret='v')
    if r.random() < 0.2 and count > 1:
        put(defs, 4 * r.randrange(1, count), 'I', 0)
    wanted = bytearray(r.choice([bytes(0x20), r.bytes(0x20)]))
    for i in range(0x20):
        if r.random() < 0.7:
            wanted[i] &= 0xfe
    regions[WANTED] = wanted
    return Call(0x51ab60, [r.getrandbits(32), WANTED, r.choice([0, 1, r.getrandbits(32)])], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
