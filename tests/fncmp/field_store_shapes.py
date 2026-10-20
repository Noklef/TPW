# The tiny functions of src/game/core/field_store_shapes (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: field_store_shapes.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put, off_code  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x409cd0, 0x409ce0, 't', 1, 'a', [], ()),
    (0x4d2a60, 0x4d2a70, 't', 1, 'a', [], (0,)),
    (0x4e0df0, 0x4e0e00, 't', 1, 'a', [], (0,)),
    (0x59b550, 0x59b590, 't', 1, 'a', [], (0,)),
    (0x5f0970, 0x5f09a0, 't', 4, 'a', [], ()),
    (0x657ddd, 0x657def, 't', 0, 'w', [], ()),
    (0x659710, 0x659730, 't', 0, 'b', [], ()),
    (0x659730, 0x659750, 't', 0, 'w', [], ()),
    (0x659790, 0x6597b0, 't', 0, 'w', [], ()),
    (0x659994, 0x6599ac, 't', 1, 'a', [], ()),
    (0x6599ac, 0x6599da, 't', 3, 'w', [], ()),
    (0x6599da, 0x6599f2, 't', 1, 'a', [], ()),
    (0x659a40, 0x659a58, 't', 1, 'a', [], ()),
    (0x659dac, 0x659de5, 't', 4, 'a', [], ()),
    (0x65f330, 0x65f345, 't', 0, 'w', [], ()),
    (0x6618c7, 0x6618e6, 't', 2, 'b', [], ()),
    (0x66190a, 0x66191b, 't', 0, 'b', [], ()),
    (0x668564, 0x66857d, 't', 1, 'a', [], ()),
    (0x668800, 0x668820, 't', 0, 'b', [], ()),
    (0x66a061, 0x66a084, 't', 2, 'w', [], ()),
    (0x66b454, 0x66b469, 't', 0, 'w', [], ()),
    (0x66b523, 0x66b53e, 't', 1, 'a', [], ()),
    (0x66c096, 0x66c0c4, 't', 3, 'w', [], ()),
    (0x66c1f3, 0x66c22c, 't', 4, 'a', [], ()),
    (0x6d2770, 0x6d2780, 's', 1, 'b', [], (0,)),
]
B = Bench([(a, e) for a, e, *_ in FUNCS])


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    off_code(obj)   # (a random dword in the exe's code would read the driver's code natively)
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
