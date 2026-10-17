# The tiny functions and wrappers of src/game/core/small4 against the originals: any object bytes, globals and arguments; the
# wage table, the global pointers, wide text for the narrow conversion through a range table.
# usage: small4.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Addr,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, stack arguments, returns, size)
CALLS = {'Addr': (0x4056f0, 't', 0, 'a', 0x8), 'V800': (0x504740, 't', 0, 'v', 0x8), 'V60': (0x429b80, 't', 0, 'v', 0x8),
         'C': (0x47a9a0, 't', 0, 'a', 0x10), 'Vb': (0x5cea30, 't', 0, 'a', 0x10), 'A36': (0x4f04c0, 't', 1, 'a', 0x10),
         'M1': (0x564a50, 'c', 0, 'a', 0x8), 'M2': (0x5ceaa0, 'c', 0, 'a', 0x8), 'M3': (0x5cea90, 's', 1, 'a', 0x8),
         'M4': (0x6bab20, 's', 4, 'a', 0x8), 'G786': (0x5d5db0, 'c', 0, 'a', 0x18), 'Wage': (0x5077f0, 't', 0, 'a', 0x10),
         'Narrow': (0x486780, 'c', 1, 'a', 0x30)}
B = Bench([(a, a + n) for (a, _c, _s, _r, n) in CALLS.values()] + [(0x5068f0, 0x50690f), (0x6ada00, 0x6ada8c), (0x6ad8f0, 0x6ad945)])
POOL = RAM + 0x3000
VALUES = [0, 1, 2, 5, 0xff, 0x100, 0x7fffffff, 0x80000000, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, nargs, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x200))
    pool = bytearray(r.bytes(0x400))
    regions = {RAM: obj, POOL: pool}
    this = RAM
    args = []
    if name == 'C':
        struct.pack_into('<I', obj, 0xc, r.choice(VALUES))
    elif name == 'Vb':
        this = RAM + 0x100
        struct.pack_into('<I', obj, 0xf8, POOL)
        struct.pack_into('<I', pool, 4, r.choice([8, 0x10, 0x18, 0x20, 0x28]))
    elif name == 'A36':
        struct.pack_into('<I', obj, 4, r.choice([0, POOL, 0x30000100]))
        args = [r.choice([0, 1, 5, 100, 0xffffffff, r.getrandbits(32)])]
    elif name in ('M3', 'M4'):
        args = [r.getrandbits(32) for _ in range(nargs)]
    elif name == 'G786':
        regions[0xf82884] = struct.pack('<I', POOL + 0x10)
        regions[0x786b80] = r.bytes(4)
    elif name == 'Wage':
        struct.pack_into('<I', obj, 0, r.randrange(0, 0x40))
        obj[8] = r.getrandbits(8)
        regions[0x785300] = r.bytes(0x1100)
    elif name == 'Narrow':
        for at_, lo in ((0, 10), (0x40, 3), (0x80, 20)):
            hi = lo + r.randrange(0, 5)
            struct.pack_into('<HHHH', pool, at_, lo, r.choice([0, 0x40, 0x80]) if at_ < 0x80 else 0, hi, r.choice([0, 0x40, 0x80]) if at_ < 0x80 else 0)
            for j in range(8):
                struct.pack_into('<H', pool, at_ + 8 + 2 * j, r.choice([0x41, 0x4142, 0x7f, 0, 0x100, 0xff00, r.getrandbits(16)]))
        struct.pack_into('<I', obj, 8, POOL + r.choice([0, 0, 0x40]))
        n = r.choice([0, 1, 2, 3, 5, 8])
        wide = [r.choice([3, 5, 10, 12, 14, 20, 22, 25, 9, 26]) for _ in range(n)] + [0, 7]
        regions[RAM + 0x1800] = struct.pack('<%dH' % len(wide), *wide)
        regions[0x78ba80] = struct.pack('<I', r.choice([0, RAM, RAM]))
        regions[0x7b5378] = bytes([0x55]) * 0x4000
        args = [RAM + 0x1800]
    call_args = args if conv == 'c' else ([this] + args if conv == 't' else args)
    return Call(address, call_args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
