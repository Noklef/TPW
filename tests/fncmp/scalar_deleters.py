# The tiny functions of src/game/core/scalar_deleters (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: scalar_deleters.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x40f270, 0x40f290, 't', 1, 'a', [], ()),
    (0x40f2b0, 0x40f2d0, 't', 1, 'a', [], ()),
    (0x40f2e0, 0x40f300, 't', 1, 'a', [], ()),
    (0x40f360, 0x40f380, 't', 1, 'a', [], ()),
    (0x40f3b0, 0x40f3d0, 't', 1, 'a', [], ()),
    (0x40f400, 0x40f420, 't', 1, 'a', [], ()),
    (0x40f450, 0x40f470, 't', 1, 'a', [], ()),
    (0x40f4a0, 0x40f4c0, 't', 1, 'a', [], ()),
    (0x40f4e0, 0x40f500, 't', 1, 'a', [], ()),
    (0x40f520, 0x40f540, 't', 1, 'a', [], ()),
    (0x40f560, 0x40f580, 't', 1, 'a', [], ()),
    (0x40f5c0, 0x40f5e0, 't', 1, 'a', [], ()),
    (0x40f610, 0x40f630, 't', 1, 'a', [], ()),
    (0x40f660, 0x40f680, 't', 1, 'a', [], ()),
    (0x40f6b0, 0x40f6d0, 't', 1, 'a', [], ()),
    (0x40f720, 0x40f740, 't', 1, 'a', [], ()),
    (0x40f780, 0x40f7a0, 't', 1, 'a', [], ()),
    (0x40f820, 0x40f840, 't', 1, 'a', [], ()),
    (0x40f890, 0x40f8b0, 't', 1, 'a', [], ()),
    (0x40f8e0, 0x40f900, 't', 1, 'a', [], ()),
    (0x40f930, 0x40f950, 't', 1, 'a', [], ()),
    (0x429b60, 0x429b80, 't', 1, 'a', [], ()),
    (0x47e3a0, 0x47e3c0, 't', 1, 'a', [], ()),
    (0x47e730, 0x47e750, 't', 1, 'a', [], ()),
    (0x4f18b0, 0x4f18d0, 't', 1, 'a', [], ()),
    (0x4ff500, 0x4ff520, 't', 1, 'a', [], ()),
    (0x5061b0, 0x5061d0, 't', 1, 'a', [], ()),
    (0x564710, 0x564730, 't', 1, 'a', [], ()),
    (0x565210, 0x565230, 't', 1, 'a', [], ()),
    (0x58b310, 0x58b330, 't', 1, 'a', [], ()),
    (0x591980, 0x5919a0, 't', 1, 'a', [], ()),
    (0x5d5c30, 0x5d5c50, 't', 1, 'a', [], ()),
    (0x617930, 0x617950, 't', 1, 'a', [], ()),
    (0x617ae0, 0x617b00, 't', 1, 'a', [], ()),
]
EXTRA = []
B = Bench([(a, e) for a, e, *_ in FUNCS] + EXTRA)


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    obj = bytearray(r.bytes(0x400))
    for k in range(0, 0x400, 4):  # many dwords point into the object, so a chain of pointers can be followed
        if r.random() < 0.5:
            put(obj, k, 'I', RAM + 0x100 + 4 * r.randrange(0x40))
    regions = {RAM: obj}
    regions[HEAP] = heap(0x1000)  # the game heap the code frees into
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
