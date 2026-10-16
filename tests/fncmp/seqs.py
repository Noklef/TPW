# The sequence helpers of src/game/core/seqs against the originals: ranges of any length, values on both sides of the limits.
# usage: seqs.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Words,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, returns, size)
CALLS = {'Words': (0x6a01a0, 's', 'a', 0x30), 'Sb': (0x6a7240, 's', 'a', 0x30), 'Br': (0x6ae2b0, 'c', 'a', 0x28),
         'Rng': (0x6ae410, 'c', 'b', 0x30), 'C4': (0x6aed80, 't', 'a', 0x20), 'C88': (0x6adf90, 't', 'a', 0x20),
         'Cp8': (0x6af730, 'c', 'a', 0x30)}
B = Bench([(a, a + n) for (a, _c, _r, n) in CALLS.values()])
POOL = RAM + 0x3000
OUT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, conv, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    pool = bytearray(r.bytes(0x400))
    regions = {RAM: obj, POOL: pool, OUT: r.bytes(0x100)}
    n = r.choice([0, 1, 2, 3, 5, 8, 16, 30])
    args = [RAM] if conv == 't' else []
    if name in ('Words', 'Sb'):
        size = 2 if name == 'Words' else 1
        args = [POOL, POOL + n * size]
    elif name == 'Br':
        pool[0:2] = struct.pack('<H', r.choice([0x28, 0x29, 0x2a, 0x27, 0, 0xffff, 0x128, 0x129]))
        args = [POOL]
    elif name == 'Rng':
        for i in range(8):
            struct.pack_into('<H', pool, 0x10 * i, r.choice([0, 0x5b0, 0x5b1, 0x5ea, 0x5eb, 0x5ec, 0x100, 0xffff, 0x15b1, r.getrandbits(16)]))
        args = [POOL, POOL + 0x10 * r.randrange(0, 9)]
    elif name == 'C4':
        struct.pack_into('<II', obj, 4, r.choice([0, POOL]), POOL + 8 * r.choice([0, 1, 5, 100]))
    elif name == 'C88':
        struct.pack_into('<II', obj, 0x88, r.choice([0, POOL]), POOL + 8 * r.choice([0, 1, 5, 100]))
        if r.random() < 0.3:
            struct.pack_into('<II', obj, 0x88, POOL + 0x40, POOL)
    elif name == 'Cp8':
        args = [POOL, POOL + 8 * n, OUT]
        if n == 0:
            ret = 'v'
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
