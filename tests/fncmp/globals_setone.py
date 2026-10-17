# The tiny functions of src/game/core/small2 against the originals: any object bytes, any globals, any arguments
# (the colour with each byte anywhere in 0-255).
# usage: small2.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Get,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, stack arguments, size)
CALLS = {'Get': (0x57ac90, 't', 0, 8), 'One1': (0x57aca0, 'c', 0, 0x14), 'Set3': (0x57cb10, 't', 3, 0x18),
         'One2': (0x57cb30, 'c', 0, 0x14), 'Ones': (0x57cb70, 'c', 0, 0x20), 'Colour': (0x5832e0, 't', 1, 0x84)}
B = Bench([(a, a + n) for (a, _c, _s, n) in CALLS.values()])
VALUES = [0, 1, 0x7f, 0x80, 0xff, 0x100, 0xff00, 0xff0000, 0xff000000, 0x80000000, 0x7fffffff, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, nargs, _n = CALLS[name]
    regions = {RAM: r.bytes(0x1000)}
    for g in (0x8bb628, 0x8bba78, 0x8bcb80, 0x8bcb84, 0x8bcb88, 0x8bcb8c):
        regions[g] = struct.pack('<I', r.choice(VALUES))
    this = RAM + (0x200 if name == 'Get' else 0)
    args = [] if conv == 'c' else [this]
    for _ in range(nargs):
        args.append(r.getrandbits(32) if name == 'Colour' and r.random() < 0.5 else r.choice(VALUES))
    return Call(address, args, regions, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
