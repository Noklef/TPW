# The tiny functions of src/game/core/list_scalar_deleters (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: list_scalar_deleters.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x41f6a0, 0x41f6f0, 't', 1, 'a', [], ()),
    (0x51aeb0, 0x51af00, 't', 1, 'a', [], ()),
    (0x51af00, 0x51af50, 't', 1, 'a', [], ()),
    (0x561a00, 0x561a30, 'c', 0, 'a', [8886816], ()),
    (0x56b760, 0x56b790, 'c', 0, 'a', [8887636], ()),
    (0x5740e0, 0x574110, 'c', 0, 'a', [8888904], ()),
    (0x580500, 0x580530, 'c', 0, 'a', [9161656], ()),
    (0x5ba7c0, 0x5ba810, 't', 1, 'a', [], ()),
    (0x5ba810, 0x5ba860, 't', 1, 'a', [], ()),
    (0x5beba0, 0x5bebf0, 't', 1, 'a', [], ()),
    (0x5bebf0, 0x5bec40, 't', 1, 'a', [], ()),
    (0x5d9750, 0x5d97a0, 't', 1, 'a', [], ()),
    (0x5da4d0, 0x5da520, 't', 1, 'a', [], ()),
    (0x5ddaf0, 0x5ddb40, 't', 1, 'a', [], ()),
    (0x5e4320, 0x5e4370, 't', 1, 'a', [], ()),
    (0x5f46e0, 0x5f4730, 't', 1, 'a', [], ()),
]
EXTRA = []
LISTY = (4322976, 5353136, 5353216,6006720, 6006800, 6024096, 6024176, 6133584, 6137040, 6150896, 6177568, 6244064)  # linked list destructors: a real chain (next at +4) is built for them
B = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    regions = {RAM: obj}
    regions[HEAP] = heap(0x1000)  # the game heap the code frees into
    if address in LISTY:
        count = r.choice([0, 1, 2, 3, 6])
        chain = bytearray(r.bytes(0x200))
        for i in range(count):
            put(chain, 0x20 * i + 4, 'I', RAM + 0x1000 + 0x20 * (i + 1) if i + 1 < count else 0)
        put(obj, 0x84, 'I', RAM + 0x1000 if count else 0)  # the object is at RAM + 0x80
        regions[RAM + 0x1000] = chain
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
