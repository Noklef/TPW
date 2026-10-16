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
    'Resolve': (0x6d48f0, 't', 1, 'a', 0x20), 'Set3': (0x5d0e30, 't', 3, 'a', 0x20), 'Set2': (0x5d0e50, 't', 2, 'a', 0x20),
    'Min1e': (0x6b6140, 't', 1, 'a', 0x28), 'Push20': (0x6b6a50, 't', 1, 'a', 0x28), 'Push24': (0x6b6a80, 't', 1, 'a', 0x28),
    'Bytes': (0x6b9900, 't', 3, 'a', 0x30), 'Past': (0x6bcb60, 't', 1, 'b', 0x28), 'AtOr': (0x6c0da0, 't', 1, 'a', 0x24),
    'S94': (0x6c51f0, 't', 2, 'a', 0x28),
    'Mv20': (0x6b6930, 't', 1, 'a', 0x40), 'Mv24': (0x6b6b00, 't', 1, 'a', 0x40), 'Page': (0x6b6b40, 't', 1, 'a', 0x60),
    'Top': (0x6b6bc0, 't', 1, 'a', 0x70), 'Ctor': (0x6bb950, 't', 0, 'a', 0x40), 'Bit400': (0x6b85a0, 't', 2, 'a', 0x58)}
B = Bench([(a, a + n) for (a, _c, _s, _r, n) in CALLS.values()] + [(0x6b6240, 0x6b6260), (0x6b6a50, 0x6b6a70), (0x6b6a80, 0x6b6aa0)])
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
    OUTP = RAM + 0x2480
    if name in ('Push20', 'Push24'):
        struct.pack_into('<I', obj, 0x20 if name == 'Push20' else 0x24, head)
    elif name == 'Past':
        obj[0x30] = r.choice([0, 1, 2, 3, 0xff, r.getrandbits(8)])
        struct.pack_into('<II', obj, 0x1c, r.choice([0, 5, 0x7fffffff, 0xfffffff0]), 0)
        struct.pack_into('<I', obj, 0x3c, r.choice([0, 3, 0x10, 0x80000000]))
    elif name == 'AtOr':
        struct.pack_into('<II', obj, 4, r.randrange(0, 5), r.choice([0, POOL, POOL]))
    regions = {RAM: obj, POOL: pool}
    if name in ('Mv20', 'Mv24'):
        n = r.randrange(1, 5)
        for i in range(n):  # a doubly linked chain at POOL + 0x40 * i (next +0x34, prev +0x38)
            struct.pack_into('<II', pool, 0x40 * i + 0x34, POOL + 0x40 * (i + 1) if i + 1 < n else 0, POOL + 0x40 * (i - 1) if i else 0)
        other = POOL + 0x300 if r.random() < 0.6 else 0
        struct.pack_into('<II', pool, 0x334, 0, 0)
        struct.pack_into('<II', obj, 0x20, POOL if name == 'Mv20' else other, other if name == 'Mv20' else POOL)
        regions[RAM + 0x4000] = struct.pack('<I', POOL + 0x40 * r.randrange(0, n))
    elif name == 'Page':
        pages = r.randrange(0, 4)
        for i in range(pages):
            struct.pack_into('<I', pool, 0x100 * i + 8, POOL + 0x100 * (i + 1) if i + 1 < pages else 0)
        struct.pack_into('<I', obj, 0x1c, POOL if pages else 0)
    elif name == 'Top':
        count = r.randrange(0, 5)
        struct.pack_into('<III', obj, 0x44, POOL, r.choice([0, 3, 5, 100]), count)
        for i in range(5):
            struct.pack_into('<I', pool, 4 * i, r.choice([0, POOL + 0x100 + 0x40 * i, POOL + 0x100 + 0x40 * i]))
            struct.pack_into('<I', pool, 0x100 + 0x40 * i + 0x20, r.choice([0, 1, 3, 5, 6, 100, 101]))
        struct.pack_into('<I', pool, 0x300 + 0x20, r.choice([0, 3, 5, 6, 100, 101, 200]))
    elif name == 'Bit400':
        struct.pack_into('<II', obj, 0x20, r.choice([0, POOL + 0x100]), r.choice([0, POOL]))
        struct.pack_into('<I', pool, 0x100 + 0x24, POOL + 0x200 if r.random() < 0.7 else 0)
        for i in range(3):
            struct.pack_into('<I', pool, 0x200 + 0x40 * i + 0x34, POOL + 0x200 + 0x40 * (i + 1) if i < 2 else 0)
            struct.pack_into('<I', pool, 0x200 + 0x40 * i + 0x30, r.choice([0, 0x400, 0x500, r.getrandbits(32)]))
    args = [RAM]
    for _ in range(nargs):
        args.append(r.choice(VALUES))
    if name == 'Min1e':
        args[1] = r.choice([0, 1, 0x1d, 0x1e, 0x1f, 0xffffffff, r.getrandbits(32)])
    elif name in ('Push20', 'Push24'):
        args[1] = POOL + 0x200
    elif name == 'Bytes':
        args = [RAM, OUTP, r.choice([0, 1, 2, 3, 4, 5, 0x100, 0x102, 0x103, 0x104, r.getrandbits(32)]), r.getrandbits(32)]
    elif name == 'Past':
        s = (struct.unpack_from('<I', obj, 0x1c)[0] + struct.unpack_from('<I', obj, 0x3c)[0]) & 0xffffffff
        args[1] = r.choice([s, (s + 1) & 0xffffffff, (s - 1) & 0xffffffff, 0, 0xffffffff, r.getrandbits(32)])
    elif name == 'AtOr':
        args[1] = r.randrange(0, 7)
    if name in ('Mv20', 'Mv24'):
        args[1] = struct.unpack_from('<I', regions[RAM + 0x4000])[0]
    elif name == 'Page':
        args[1] = r.choice([0, 1, 99, 100, 101, 199, 200, 250, 299, 300, 399, 0xffff, 0x10064, r.randrange(0, 450)])
    elif name == 'Top':
        args[1] = r.choice([0, POOL + 0x300, POOL + 0x300, POOL + 0x300])
    elif name == 'Bit400':
        args = [RAM, OUTP, r.choice([0, 1, 0xff])]
    if name == 'Find' and nodes and r.random() < 0.7:
        args[1] = struct.unpack_from('<I', pool, 0x40 * r.randrange(nodes) + 0x2c)[0]
    if name == 'Resolve':
        args[1] = r.randrange(0, 8)
    return Call(address, args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
