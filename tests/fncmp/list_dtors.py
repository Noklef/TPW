# The tiny functions of src/game/core/list_dtors (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: list_dtors.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x407c30, 0x407c70, 't', 0, 'v', [], ()),
    (0x41c9a0, 0x41c9e0, 't', 0, 'v', [], ()),
    (0x41eb50, 0x41eb90, 't', 0, 'v', [], ()),
    (0x421640, 0x421680, 't', 0, 'v', [], ()),
    (0x5193d0, 0x519410, 't', 0, 'v', [], ()),
    (0x5ac550, 0x5ac590, 't', 0, 'v', [], ()),
    (0x5ac590, 0x5ac5d0, 't', 0, 'v', [], ()),
    (0x5b58c0, 0x5b5900, 't', 0, 'v', [], ()),
    (0x5b9a80, 0x5b9ac0, 't', 0, 'v', [], ()),
    (0x5b9ac0, 0x5b9b00, 't', 0, 'v', [], ()),
    (0x5beae0, 0x5beb20, 't', 0, 'v', [], ()),
    (0x5beb20, 0x5beb60, 't', 0, 'v', [], ()),
    (0x5beb60, 0x5beba0, 't', 0, 'v', [], ()),
    (0x5d5800, 0x5d5840, 't', 0, 'v', [], ()),
    (0x5d8610, 0x5d8650, 't', 0, 'v', [], ()),
    (0x5d8650, 0x5d8690, 't', 0, 'v', [], ()),
    (0x5d9f10, 0x5d9f50, 't', 0, 'v', [], ()),
    (0x5da2b0, 0x5da2f0, 't', 0, 'v', [], ()),
    (0x5dc6c0, 0x5dc700, 't', 0, 'v', [], ()),
    (0x5dc700, 0x5dc740, 't', 0, 'v', [], ()),
    (0x5e0370, 0x5e03b0, 't', 0, 'v', [], ()),
    (0x5e3cd0, 0x5e3d10, 't', 0, 'v', [], ()),
    (0x5e40a0, 0x5e40e0, 't', 0, 'v', [], ()),
    (0x5f3900, 0x5f3940, 't', 0, 'v', [], ()),
]
EXTRA = []
LISTY = (4226096, 4311456, 4320080, 4331072, 5346256, 5948752, 5948816, 5986496, 6003328, 6003392, 6023904, 6023968, 6024032, 6117376, 6129168, 6129232, 6135568, 6136496, 6145728, 6145792, 6161264, 6175952, 6176928, 6240512)  # linked list destructors: a real chain (next at +4) is built for them
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
