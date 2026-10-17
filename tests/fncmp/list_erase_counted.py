# The counted list range erase and nth erase, the table lookup with a limit and the activity check of src/game/core/list_erase_counted against the
# originals: any object bytes and arguments, the table of records at 0x76e300 filled at run time by the game.
# usage: list_erase_counted.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

CALLS = {'Range': (0x5cdfe0, 't', 'a', 0xa0), 'Limit': (0x59c570, 't', 'b', 0xa0), 'Act': (0x5b6230, 't', 'a', 0xb0), 'Nth': (0x5cd660, 't', 'b', 0xb0)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1)])
LIST, BUFS, TABLE = RAM + 0x1000, RAM + 0x2000, 0x76e300
NREC = 342


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x800))
    regions = {RAM: obj, HEAP: heap()}
    if name in ('Range', 'Nth'):
        n = r.choice([0, 1, 2, 3, 4])
        nodes = [LIST + 0x30 * (i + 1) for i in range(n)]
        order = [LIST] + nodes
        data = bytearray(r.bytes(0x30 * (n + 1)))
        bufs = bytearray(r.bytes(0x200))
        for i, a in enumerate(order):
            put(data, a - LIST, 'II', order[(i + 1) % len(order)], order[i - 1])
            bufs[0x30 * i + 2] = r.choice([0, 1, 2, 5, 0xfe, 0xff])
            put(data, a - LIST + 0xc, 'I', r.choice([0, BUFS + 0x30 * i + 3, BUFS + 0x30 * i + 3]))
        regions.update({LIST: data, BUFS: bufs})
        put(obj, 4, 'I', LIST)
        put(obj, 8, 'I', r.choice([n, n + 1, 0, 100]))
        if name == 'Nth':
            return Call(address, [RAM, r.choice([0, 1, 2, 3, 4, 5, 0xffffffff])], regions, conv='t', ret=ret)
        return Call(address, [RAM, RAM + 0x300, r.choice(order), r.choice(order)], regions, conv='t', ret=ret)
    if name == 'Act':
        # the nine blocks pointed to from +0xe0..+0x100, each with a dword at +0xc that is mostly 0
        blocks = bytearray(r.bytes(0x100))
        for k in range(9):
            put(obj, 0xe0 + 4 * k, 'I', RAM + 0x400 + 0x10 * k)
            put(blocks, 0x10 * k + 0xc, 'I', r.choice([0, 0, 0, 0, 0, 0, 0, 1, 0xffffffff]))
        regions[RAM + 0x400] = blocks
        for off in (0, 0x6c, 0x324, 0x38c):
            put(obj, off, 'I', r.choice([0, 0, 0, 0, 0, 3]))
        regions[RAM] = obj
        return Call(address, [RAM], regions, conv='t', ret=ret)
    table = bytearray(r.bytes(56 * NREC + 0x40))
    for i in range(NREC):
        put(table, 56 * i + 4, 'I', i if r.random() < 0.7 else r.choice([0, 1, 5, 17, r.randrange(NREC)]))
        put(table, 56 * i + 8, 'I', r.choice([0, 1, 2, 3, 10, 10]))
    for k in range(11):
        put(obj, 0x2c + 12 * k, 'i', r.choice([0, 0, 1, 5, 100, -3, 0x7fffffff]))
    regions[TABLE] = table
    return Call(address, [RAM, r.choice([0, 1, 2, 5, 17, 100, 0x15e, 0x15f, 0x160, 0x161, 200, 300, 341]),
                          r.choice([0, 1, 4, 5, 99, 100, 101, -1, -100, 0x7fffffff, 0x80000000])], regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
