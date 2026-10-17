# The tiny functions of src/game/core/globals_vtables against the originals: any global values, any argument, objects whose neighbours
# before the start hold vtable pointers with small offsets, list cursors.
# usage: globals_vtables.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Eq,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, size)
CALLS = {'Eq': (0x52f860, 'c', 0x14), 'If1': (0x52fab0, 'c', 0x14), 'If2': (0x52faf0, 'c', 0x14), 'Add': (0x52fba0, 'c', 0x14),
         'Rd': (0x557d60, 'c', 0x20), 'G879': (0x573860, 'c', 0x10), 'V8': (0x4f0140, 't', 0x10), 'V4': (0x5cde30, 't', 0x10),
         'Next': (0x5ce910, 't', 0x10)}
B = Bench([(a, a + n) for (a, _c, n) in CALLS.values()])
POOL = RAM + 0x3000
VALUES = [0, 1, 2, 5, 0x7f, 0x80, 0xff, 0x100, 0x7fffffff, 0x80000000, 0xffffffff]
GLOBALS = (0x81ae2c, 0x8187f8, 0x81b0cc, 0x81ecac, 0x81af4c)


def case(r):
    name = r.choice(OPS)
    address, conv, _n = CALLS[name]
    obj = bytearray(r.bytes(0x200))
    pool = bytearray(r.bytes(0x400))
    regions = {RAM: obj, POOL: pool}
    for g in GLOBALS:
        regions[g] = struct.pack('<I', r.choice(VALUES))
    args = []
    if name == 'Eq':
        args = [struct.unpack('<I', regions[0x81ae2c])[0] if r.random() < 0.5 else r.choice(VALUES)]
    elif name == 'Add':
        args = [r.choice(VALUES)]
    elif name == 'Rd':
        regions[RAM + 0x800] = r.bytes(0x10)
        args = [RAM, RAM + 0x800, RAM + 0x804, RAM + 0x808 if r.random() < 0.8 else RAM + 0x804]
    elif name == 'G879':
        regions[0x879a24] = struct.pack('<I', POOL - 0x4c4 + 0x40)
    elif name in ('V8', 'V4'):
        d = 8 if name == 'V8' else 4
        this = RAM + 0x100
        struct.pack_into('<I', obj, 0x100 - d, POOL)           # the vtable pointer
        struct.pack_into('<I', pool, 4, r.choice([d, d + 8, d + 0x10, 0x20 + d]))  # the offset: the target lies in the object
        args = [this]
    elif name == 'Next':
        struct.pack_into('<I', obj, 0x100, POOL + 0x10 * r.randrange(0, 4))
        for i in range(4):
            struct.pack_into('<I', pool, 0x10 * i, r.choice([POOL + 0x10 * r.randrange(0, 4), 0]))
        args = [RAM + 0x100, RAM + 0x180, r.choice([0, 1, 0xffffffff])]
    return Call(address, args if conv == 'c' else args, regions, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
