# The indexed getters and setters of src/game/core/indexed against the originals: any object bytes and game global, indexes
# small and wild, vector pointers that are unset, equal or apart.
# usage: indexed.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, stack arguments, returns, size)
CALLS = {'Set': (0x41a960, 't', 0, 'a', 0x10), 'Minus': (0x41a970, 't', 0, 'a', 0x14), 'MinusQ': (0x41a990, 't', 0, 'a', 0x17), 'Idx4e0': (0x4145d0, 't', 1, 'a', 0x10),
         'W6c': (0x4e0aa0, 't', 1, 'a', 0x14), 'Clr28': (0x5099a0, 't', 1, 'a', 0x14), 'Get28': (0x509990, 't', 1, 'a', 0x14),
         'Count': (0x5ed430, 't', 0, 'a', 0x24)}
B = Bench([(a, a + n) for (a, _c, _s, _r, n) in CALLS.values()])
POOL = RAM + 0x3000
VALUES = [0, 1, 2, 3, 5, 0x7f, 0x80, 0xff, 0x100, 0xffff, 0x10000, 0x7fffffff, 0x80000000, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, nargs, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x600))
    for off in (0, 4, 8, 0x6c, 0x28, 0x34):
        if r.random() < 0.6:
            struct.pack_into('<I', obj, off, r.choice(VALUES))
    game = bytearray(r.bytes(0x20))
    # the game object is only read at 0x1da70c: a small window there (the global points 0x1da70c below it)
    regions = {RAM: obj, POOL: game, 0x80239c: struct.pack('<I', POOL - 0x1da70c)}
    args = [RAM]
    if name == 'Count':
        begin = r.choice([0, POOL, POOL + 0x14])
        struct.pack_into('<II', obj, 4, begin, r.choice([POOL, POOL + 0x14 * r.randrange(0, 6), POOL - 0x28, POOL + 0x10]))
    for _ in range(nargs):
        if name in ('Clr28', 'Get28', 'Idx4e0'):
            args.append(r.randrange(0, 0x20))
        elif name == 'W6c':
            w = struct.unpack_from('<H', obj, 0x6c)[0]
            args.append(w | r.choice([0, 0x10000, 0xabcd0000]) if r.random() < 0.6 else r.choice(VALUES))
        else:
            args.append(r.choice(VALUES))
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
