# The tiny functions of src/game/core/tiny_f (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: tiny_f.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x402df0, 0x402e00, 't', 0, 'a', [], ()),
    (0x40f220, 0x40f230, 'c', 0, 'a', [], ()),
    (0x454540, 0x454550, 'c', 0, 'a', [], ()),
    (0x481590, 0x4815a0, 'c', 0, 'a', [], ()),
    (0x4815a0, 0x4815b0, 'c', 0, 'a', [], ()),
    (0x4815b0, 0x4815c0, 's', 1, 'a', [], ()),
    (0x5f5640, 0x5f5650, 'c', 0, 'a', [], ()),
    (0x5f5650, 0x5f5660, 'c', 1, 'a', [], (0,)),
    (0x691ed0, 0x691ee0, 'c', 0, 'a', [], ()),
    (0x691f20, 0x691f30, 'c', 0, 'a', [], ()),
    (0x692100, 0x692110, 'c', 0, 'a', [], ()),
    (0x692960, 0x692970, 'c', 0, 'a', [], ()),
    (0x6929b0, 0x6929c0, 'c', 0, 'a', [], ()),
    (0x6929f0, 0x692a00, 'c', 0, 'a', [], ()),
    (0x6baa10, 0x6baa20, 's', 1, 'a', [], ()),
    (0x6c4900, 0x6c4910, 'c', 0, 'a', [], ()),
]
EXTRA = [(4206320, 4206352), (4206784, 4206800), (4231776, 4231840), (4345552, 4345584), (4770080, 4770096), (4770096, 4770112), (4770112, 4770128), (6391280, 6391296), (6391296, 6391312), (6505664, 6505712), (6889184, 6889200), (6891888, 6891904), (7098288, 7098320), (7150528, 7150544)]
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
