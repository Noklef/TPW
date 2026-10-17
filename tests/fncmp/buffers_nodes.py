# The tiny allocating and clearing functions of src/game/core/buffers_nodes against the originals: any object bytes and arguments
# (the allocator is the heap region), objects at addresses with different top bytes.
# usage: buffers_nodes.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap  # noqa: E402

# name: (address, convention, stack arguments, returns, size)
CALLS = {'Nop': (0x46cfb0, 'c', 0, 'v', 0x10), 'FreeIf': (0x47b1a0, 't', 0, 'v', 0x20), 'FreeClr': (0x47b170, 't', 0, 'a', 0x30),
         'N28': (0x5b2fd0, 's', 2, 'a', 0x20), 'N18': (0x5b1680, 's', 2, 'a', 0x30), 'Head': (0x5cd400, 't', 0, 'a', 0x30),
         'A3Init': (0x5d3830, 't', 3, 'a', 0x30), 'A3Free': (0x5d3860, 't', 0, 'v', 0x30), 'Del': (0x569c80, 't', 1, 'a', 0x30)}
B = Bench([(a, a + n) for (a, _c, _s, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1), (0x565230, 0x565240)])
VALUES = [0, 1, 2, 5, 0x10, 0xff, 0x100, 0x7fffffff, 0x80000000, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, nargs, ret, _n = CALLS[name]
    base = r.choice([RAM, RAM, 0x3a000000, 0x1d000000]) if name == 'Head' else RAM
    obj = bytearray(r.bytes(0x100))
    regions = {base: obj, HEAP: heap()}
    this = base + 0x40 if name == 'Del' else base
    args = []
    if name in ('FreeIf', 'FreeClr', 'A3Free'):
        # the block pointer: null, or an address the allocator handed out before (any value frees the same in both)
        struct.pack_into('<I', obj, 4 if name != 'A3Free' else 0xc, r.choice([0, HEAP + 0x40, RAM + 0x80, r.getrandbits(32)]))
        if name == 'FreeIf':
            obj[0] = r.choice([0, 1, 2, 0xff])
    elif name in ('N28', 'N18'):
        args = [r.choice(VALUES), r.choice(VALUES)]
    elif name == 'A3Init':
        args = [r.choice([0, 1, 3, 16, 0x100, 0x1000]) for _ in range(3)]
    elif name == 'Del':
        args = [r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])]
    call_args = args if conv == 'c' or conv == 's' else [this] + args
    return Call(address, call_args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
