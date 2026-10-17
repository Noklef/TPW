# The tiny functions of src/game/core/tiny_h (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: tiny_h.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x6b0140, 0x6b0150, 'c', 0, 'a', [], ()),
    (0x6b0180, 0x6b0190, 'c', 0, 'a', [], ()),
    (0x6b01c0, 0x6b01d0, 'c', 0, 'a', [], ()),
    (0x6b0200, 0x6b0210, 'c', 0, 'a', [], ()),
]
EXTRA = [(7014160, 7014192), (7020000, 7020032), (7023120, 7023152), (7027712, 7027744), (7031392, 7031408)]
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
