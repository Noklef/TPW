# The tiny functions of src/game/core/framestubs (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: framestubs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched)
FUNCS = [
    (0x64fe7d, 0x64fe82, 'c', 0, 'v', []),
    (0x6503c1, 0x6503cb, 'c', 0, 'a', []),
    (0x6503cb, 0x6503d0, 'c', 0, 'v', []),
    (0x654a1d, 0x654a22, 'c', 0, 'v', []),
    (0x655e8d, 0x655ea0, 'c', 0, 'v', []),
    (0x657def, 0x657dff, 's', 1, 'w', []),
    (0x658490, 0x6584a0, 's', 3, 'w', []),
    (0x658aab, 0x658ab4, 'c', 0, 'w', []),
    (0x658cc5, 0x658ccc, 'c', 0, 'a', []),
    (0x6598dd, 0x6598ea, 's', 3, 'v', []),
    (0x6598ea, 0x6598f7, 's', 4, 'v', []),
    (0x659985, 0x659994, 's', 2, 'a', []),
    (0x659b49, 0x659b52, 'c', 0, 'w', []),
    (0x65ad33, 0x65ad43, 'c', 0, 'a', []),
    (0x65bc60, 0x65bc70, 'c', 0, 'a', []),
    (0x65c2c8, 0x65c2d8, 'c', 0, 'a', []),
    (0x65dd5f, 0x65dd6c, 's', 1, 'v', []),
    (0x65dd6c, 0x65dd7b, 'c', 0, 'w', []),
    (0x661596, 0x6615a3, 'c', 0, 'a', []),
    (0x6618ba, 0x6618c7, 's', 2, 'v', []),
    (0x66191b, 0x661928, 's', 2, 'v', []),
    (0x661928, 0x661935, 's', 1, 'v', []),
    (0x661b33, 0x661b3c, 'c', 0, 'w', []),
    (0x661c2b, 0x661c36, 'c', 0, 'v', []),
    (0x661e60, 0x661e70, 's', 1, 'v', []),
    (0x6628e4, 0x6628f4, 'c', 0, 'a', []),
    (0x6662a7, 0x6662b7, 'c', 0, 'a', []),
    (0x66825c, 0x668269, 's', 2, 'v', []),
    (0x668aad, 0x668abd, 'c', 0, 'a', []),
    (0x668c89, 0x668c99, 's', 2, 'w', []),
    (0x66a040, 0x66a04b, 'c', 0, 'v', []),
    (0x66a04b, 0x66a056, 'c', 0, 'v', []),
    (0x66a056, 0x66a061, 'c', 0, 'v', []),
    (0x66a127, 0x66a132, 'c', 0, 'v', []),
    (0x66a132, 0x66a13d, 'c', 0, 'v', []),
    (0x66a13d, 0x66a148, 'c', 0, 'v', []),
    (0x66a271, 0x66a281, 'c', 0, 'a', []),
    (0x66a5b4, 0x66a5c4, 'c', 0, 'a', []),
    (0x66ae73, 0x66ae83, 'c', 0, 'a', []),
    (0x66bfc0, 0x66bfd0, 's', 2, 'w', []),
    (0x66bfd0, 0x66bfdd, 's', 2, 'v', []),
    (0x66bfdd, 0x66bfea, 's', 2, 'v', []),
    (0x66bfea, 0x66bff7, 's', 4, 'v', []),
    (0x66c00b, 0x66c01a, 's', 1, 'a', []),
    (0x66c156, 0x66c163, 's', 4, 'v', []),
    (0x66c2d2, 0x66c2df, 's', 4, 'v', []),
    (0x66c744, 0x66c760, 's', 4, 'v', []),
    (0x66c84a, 0x66c85a, 'c', 0, 'a', []),
    (0x66cac3, 0x66cad3, 'c', 0, 'a', []),
    (0x66d1e8, 0x66d1f8, 'c', 0, 'a', []),
    (0x66d644, 0x66d651, 's', 1, 'v', []),
    (0x66d6ea, 0x66d6f5, 'c', 0, 'v', []),
    (0x66d875, 0x66d885, 'c', 0, 'a', []),
    (0x66da3d, 0x66da4d, 'c', 0, 'a', []),
]
B = Bench([(a, e) for a, e, *_ in FUNCS])


def case(r):
    address, _end, conv, n, ret, globs = r.choice(FUNCS)
    regions = {RAM: r.bytes(0x400)}
    for g in globs:
        regions[g & ~0xf] = r.bytes(0x20)
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    if conv == 't':
        args = [RAM + 0x80] + args
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
