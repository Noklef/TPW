# The tiny functions of src/game/core/thunks_vtbl (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: thunks_vtbl.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x492020, 0x492040, 't', 0, 'a', [], ()),
    (0x492060, 0x492070, 't', 0, 'a', [], ()),
    (0x4fa460, 0x4fa470, 't', 0, 'a', [], ()),
    (0x6b0470, 0x6b0480, 't', 0, 'v', [], ()),
    (0x6b0710, 0x6b0730, 't', 0, 'a', [], ()),
    (0x6b0750, 0x6b0760, 't', 0, 'v', [], ()),
    (0x6b1de0, 0x6b1e00, 't', 0, 'a', [], ()),
    (0x6b1e20, 0x6b1e30, 't', 0, 'v', [], ()),
    (0x6b2a10, 0x6b2a30, 't', 0, 'a', [], ()),
    (0x6b2a50, 0x6b2a60, 't', 0, 'v', [], ()),
    (0x6b3c00, 0x6b3c20, 't', 0, 'a', [], ()),
    (0x6b3c40, 0x6b3c50, 't', 0, 'v', [], ()),
]
EXTRA = [(4331504, 4331520), (6725544, 6725614), (6725614, 6725634), (7031376, 7031392), (7031392, 7031408), (7031440, 7031456)]
B = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    regions = {RAM: obj}
    for g in globs:
        regions[g & ~0xf] = r.bytes(0x20)
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    for i in ptr:
        args[i] = RAM + 0x200 + 0x40 * min(i, 3) + r.choice([0, 4, 0x20])
    if conv == 't':
        args = [RAM + 0x80] + args
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
