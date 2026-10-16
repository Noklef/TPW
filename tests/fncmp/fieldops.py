# The field setters, selectors and list walkers of src/game/core/fieldops against the originals: any object bytes with
# pointers set up to point into the buffers (sometimes to each other), chains of nodes ending or not, any arguments.
# usage: fieldops.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Flag80,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# name: (address, convention, stack arguments, returns, size)
CALLS = {
    'Flag80': (0x6c5190, 't', 1, 'a', 0x10), 'Flag8': (0x6c51b0, 't', 1, 'a', 0x10), 'Byte44': (0x6b7130, 't', 1, 'a', 0x12),
    'Times8': (0x6cc760, 't', 1, 'a', 0x14), 'Pair54': (0x6b9c90, 't', 2, 'a', 0x18), 'Rate': (0x6c6fd0, 't', 0, 'a', 0x20),
    'Word6': (0x6b0560, 't', 0, 'a', 0x20), 'Byte5': (0x6b05e0, 't', 0, 'a', 0x20), 'Promote': (0x6b6240, 't', 0, 'v', 0x20),
    'Find': (0x6b6970, 't', 1, 'a', 0x20), 'Nibble': (0x6b54a0, 't', 0, 'a', 0x20), 'Pop': (0x56afa0, 't', 0, 'a', 0x20),
    'Resolve': (0x6d48f0, 't', 1, 'a', 0x20), 'Set3': (0x5d0e30, 't', 3, 'a', 0x20), 'Set2': (0x5d0e50, 't', 2, 'a', 0x20)}
B = Bench([(a, a + n) for (a, _c, _s, _r, n) in CALLS.values()])
VALUES = [0, 1, 2, 3, 0x7f, 0x80, 0xff, 0x100, 0x400, 0x500, 0x7fffffff, 0x80000000, 0xffffffff, 0xfffffbff]
POOL = RAM + 0x3000


def case(r):
    name = r.choice(OPS)
    address, conv, nargs, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x2500))
    for off in range(0, 0x80, 4):
        if r.random() < 0.5:
            struct.pack_into('<I', obj, off, r.choice(VALUES))
    pool = bytearray(r.bytes(0x400))
    nodes = r.randrange(0, 6)
    for i in range(nodes):  # a chain of nodes 0x40 apart: flags +0x30, key +0x2c, next +0x34
        struct.pack_into('<III', pool, 0x40 * i + 0x2c, r.choice([0, 1, 5, 5, 0x77]), r.choice(VALUES), POOL + 0x40 * (i + 1) if i + 1 < nodes else 0)
    head = POOL if nodes else 0
    if name == 'Promote':
        struct.pack_into('<I', obj, 0x24, head)
    elif name == 'Find':
        struct.pack_into('<I', obj, 0x20, head)
    elif name in ('Word6', 'Byte5'):
        struct.pack_into('<II', obj, 8, r.choice([0, 1]), POOL + r.choice([0, 0x10, 0x11]))
    elif name == 'Nibble':
        struct.pack_into('<I', obj, 4, POOL + r.choice([0, 1, 0x20]))
        obj[0] = r.choice([0, 1, 0xff])
    elif name == 'Pop':
        struct.pack_into('<i', obj, 0x200, r.choice([-1, 0, 1, 2, 5, 0x20]))
    elif name == 'Resolve':
        count = r.randrange(0, 6)
        struct.pack_into('<II', obj, 8, POOL, count)
    elif name in ('Set3', 'Set2'):
        slots = [RAM + 0x2400 + 4 * r.randrange(0, 6) for _ in range(3)]
        if r.random() < 0.3:  # the targets overlap
            slots[1] = slots[0]
        for off, slot in zip((0xc, 0x1c, 0x2c) if name == 'Set3' else (0x10, 0x20, 0x30), slots):
            struct.pack_into('<I', obj, off, slot)
    regions = {RAM: obj, POOL: pool}
    args = [RAM]
    for _ in range(nargs):
        args.append(r.choice(VALUES))
    if name == 'Find' and nodes and r.random() < 0.7:
        args[1] = struct.unpack_from('<I', pool, 0x40 * r.randrange(nodes) + 0x2c)[0]
    if name == 'Resolve':
        args[1] = r.randrange(0, 8)
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
