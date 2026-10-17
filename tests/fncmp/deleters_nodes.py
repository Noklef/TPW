# The tiny deleting destructors, constructors and clears of src/game/core/deleters_nodes against the originals: any object bytes, flags and
# arguments (the allocator is the heap region).
# usage: deleters_nodes.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, HEAP, RAM, heap, put  # noqa: E402

# name: (address, convention, stack arguments, returns, size)
CALLS = {'Del60': (0x42a090, 't', 1, 'a', 0x30), 'Del8': (0x56a660, 't', 1, 'a', 0x30), 'FreeC': (0x55f1e0, 't', 0, 'v', 0x30),
         'Ctor58': (0x5c8d80, 't', 0, 'a', 0x30), 'From': (0x5cde50, 't', 1, 'a', 0x30), 'N0c': (0x5d0690, 's', 2, 'a', 0x30),
         'A3x4': (0x5eb170, 't', 0, 'v', 0x30), 'S118': (0x4d05d0, 't', 1, 'v', 0x30)}
B = Bench([(a, a + n) for (a, _c, _s, _r, n) in CALLS.values()] + [(0x5da3c0, 0x5da3c1), (0x565230, 0x565240), (0x5c9040, 0x5c9062), (0x5d36c0, 0x5d36d0),
                                                                    (0x5d3860, 0x5d3890)])
VALUES = [0, 1, 2, 5, 0x10, 0xff, 0x100, 0x7fffffff, 0x80000000, 0xffffffff]


def case(r):
    name = r.choice(OPS)
    address, conv, nargs, ret, _n = CALLS[name]
    obj = bytearray(r.bytes(0x800))
    regions = {RAM: obj, HEAP: heap()}
    this = RAM + 0x80
    args = []
    if name in ('Del60', 'Del8', 'S118'):
        args = [r.choice([0, 1, 2, 3, 0xffffffff, r.getrandbits(32)])]
    elif name == 'FreeC':
        put(obj, 0x80 + 0xc, 'I', r.choice([0, HEAP + 0x40, RAM + 0x100]))
    elif name == 'From':
        regions[RAM + 0x400] = r.bytes(8)
        args = [RAM + 0x400]
    elif name == 'N0c':
        return Call(address, [r.choice(VALUES), r.choice(VALUES)], regions, conv='s', ret=ret)
    elif name == 'A3x4':
        for off in (0x2c, 0x3c, 0x4c, 0x5c):
            put(obj, 0x80 + off + 0xc, 'I', r.choice([0, 0, HEAP + 0x40, RAM + 0x100]))
    return Call(address, [this] + args, regions, conv=conv, ret=ret)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
