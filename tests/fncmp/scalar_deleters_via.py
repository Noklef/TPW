# The tiny functions of src/game/core/scalar_deleters_via (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: scalar_deleters_via.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x407c70, 0x407c90, 't', 1, 'a', [], ()),
    (0x41c980, 0x41c9a0, 't', 1, 'a', [], ()),
    (0x41eb30, 0x41eb50, 't', 1, 'a', [], ()),
    (0x421620, 0x421640, 't', 1, 'a', [], ()),
    (0x492040, 0x492060, 't', 1, 'a', [], ()),
    (0x4e5bc0, 0x4e5be0, 't', 1, 'a', [], ()),
    (0x5193b0, 0x5193d0, 't', 1, 'a', [], ()),
    (0x5ac530, 0x5ac550, 't', 1, 'a', [], ()),
    (0x5b5900, 0x5b5920, 't', 1, 'a', [], ()),
    (0x5b9a40, 0x5b9a60, 't', 1, 'a', [], ()),
    (0x5b9a60, 0x5b9a80, 't', 1, 'a', [], ()),
    (0x5bea80, 0x5beaa0, 't', 1, 'a', [], ()),
    (0x5beaa0, 0x5beac0, 't', 1, 'a', [], ()),
    (0x5beac0, 0x5beae0, 't', 1, 'a', [], ()),
    (0x5d57e0, 0x5d5800, 't', 1, 'a', [], ()),
    (0x5d85d0, 0x5d85f0, 't', 1, 'a', [], ()),
    (0x5d85f0, 0x5d8610, 't', 1, 'a', [], ()),
    (0x5d9ef0, 0x5d9f10, 't', 1, 'a', [], ()),
    (0x5da290, 0x5da2b0, 't', 1, 'a', [], ()),
    (0x5dc680, 0x5dc6a0, 't', 1, 'a', [], ()),
    (0x5dc6a0, 0x5dc6c0, 't', 1, 'a', [], ()),
    (0x5e0350, 0x5e0370, 't', 1, 'a', [], ()),
    (0x5e3cb0, 0x5e3cd0, 't', 1, 'a', [], ()),
    (0x5e4080, 0x5e40a0, 't', 1, 'a', [], ()),
    (0x5f38e0, 0x5f3900, 't', 1, 'a', [], ()),
    (0x6175f0, 0x617610, 't', 1, 'a', [], ()),
]
EXTRA = [(4226096, 4226160), (4311456, 4311520), (4320080, 4320144), (4331072, 4331136), (4711312, 4711328), (4792416, 4792432), (5135248, 5135296), (5346256, 5346320), (5948752, 5948816), (5986496, 5986560), (6003328, 6003392), (6003392, 6003456), (6023904, 6023968), (6023968, 6024032), (6024032, 6024096), (6117376, 6117440), (6129168, 6129232), (6129232, 6129296), (6135568, 6135632), (6136496, 6136560), (6145728, 6145792), (6145792, 6145856), (6161264, 6161328), (6175952, 6176016), (6176928, 6176992), (6240512, 6240576), (6725614, 6725634)]
LISTY = (4226160, 4311424, 4320048, 4331040, 5346224, 5948720, 5986560, 6003264, 6003296, 6023808, 6023840, 6023872, 6117344, 6129104, 6129136, 6135536, 6136464, 6145664, 6145696, 6161232, 6175920, 6176896, 6240480)  # linked list destructors: a real chain (next at +4) is built for them
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
