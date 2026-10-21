# The table lookups and cursors of src/game/core/cursors against the originals: tables with the base unset, indexes inside
# and outside the limit, packed bytes either side of the limit, cursors at each phase.
# usage: cursors.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=T18,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, returns, size)
CALLS = {'T18': (0x6b0620, 'a', 0x30), 'T': (0x6b0650, 'a', 0x30), 'Pk': (0x6b0510, 'w', 0x48), 'NW': (0x6b5450, 'a', 0x50),
         'NN': (0x6b54c0, 'a', 0x30), 'BN': (0x6b54f0, 'a', 0x48),
         'Cp': (0x6b5540, 'a', 0x20)}
B = Bench([(a, a + n) for (a, _r, n) in CALLS.values()])
POOL = RAM + 0x3000
OUT = RAM + 0x1000


def case(r):
    name = r.choice(OPS)
    address, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x100))
    pool = bytearray(r.bytes(0x400))
    regions = {RAM: obj, POOL: pool, OUT: r.bytes(8)}
    args = [RAM]
    if name in ('T18', 'T'):
        struct.pack_into('<III', obj, 8, r.choice([0, 0x1000, 0xfffff000]), POOL, POOL + 0x100)
        struct.pack_into('<H', pool, 6, r.choice([0, 1, 3, 7]))
        args += [r.choice([0, 1, 3, 4, 7, 8, 0x10000, 0x10003, 0xffff, r.randrange(0, 10)])]
    elif name == 'Pk':
        struct.pack_into('<I', obj, 0x14, POOL + 0x200)
        pool[0x208] = r.choice([0, 1, 2, 5, 0x80, 0xff])
        cursor = POOL + 0x40 * r.randrange(0, 4)
        pool[cursor - POOL] = r.choice([0, 1, 2, 3, 5, 6, 0x7f, 0x80, 0xff, r.getrandbits(8)])
        regions[RAM + 0x800] = struct.pack('<I', cursor)
        args += [RAM + 0x800]
    elif name == 'NW':
        struct.pack_into('<I', obj, 0, POOL + 0x10 * r.randrange(0, 4))
        obj[4] = r.choice([0, 1, 0xff])
        obj[5] = r.choice([0, 0, 0, 1])
        struct.pack_into('<I', obj, 8, r.choice([0, 1, 0xffffffff, r.getrandbits(32)]))
        args += [r.choice([0, 1, 0xf, 0x10, 0xab, 0xffffffff, r.getrandbits(32)])]
    elif name == 'NN':
        obj[0] = r.choice([0, 1, 0xff])
        struct.pack_into('<I', obj, 4, POOL + r.randrange(0, 0x100))
        args += [OUT, r.choice([0, 1, 5])]
    elif name == 'BN':
        obj[0] = r.choice([1, 2, 4, 0x40, 0x80, 0x01, 0, 0xff, r.getrandbits(8)])
        struct.pack_into('<I', obj, 4, POOL + r.randrange(0, 0x100))
        args += [OUT, r.choice([0, 1, 5])]
    elif name == 'Cp':
        args += [OUT]
    return Call(address, args, regions, conv='t', ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
