# The tiny functions of src/game/core/tiny_c (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: tiny_c.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, first argument a pointer)
FUNCS = [
    (0x402f30, 0x402f40, 't', 0, 'a', [], False),
    (0x4053b0, 0x4053c0, 't', 0, 'a', [], False),
    (0x421510, 0x421520, 't', 0, 'a', [], False),
    (0x4cf030, 0x4cf050, 't', 0, 'a', [], False),
    (0x4d0f70, 0x4d0f90, 't', 0, 'a', [], False),
    (0x4d3ed0, 0x4d3ee0, 't', 0, 'a', [], False),
    (0x4e0370, 0x4e0380, 't', 0, 'a', [], False),
    (0x4e0380, 0x4e0390, 't', 0, 'a', [], False),
    (0x4e03e0, 0x4e03f0, 't', 0, 'a', [], False),
    (0x4e0440, 0x4e0450, 't', 0, 'a', [], False),
    (0x4ef830, 0x4ef840, 't', 0, 'a', [], False),
    (0x4f06a0, 0x4f06b0, 's', 1, 'w', [], False),
    (0x4f06b0, 0x4f06c0, 'c', 0, 'w', [], False),
    (0x4f1890, 0x4f18b0, 't', 1, 'a', [], False),
    (0x4f1bd0, 0x4f1be0, 't', 0, 'a', [], False),
    (0x513290, 0x5132a0, 't', 0, 'a', [], False),
    (0x5ac510, 0x5ac530, 't', 0, 'a', [], False),
    (0x5cfd80, 0x5cfd90, 't', 0, 'a', [], False),
    (0x5d36c0, 0x5d36d0, 't', 0, 'a', [], False),
    (0x5ec210, 0x5ec220, 't', 0, 'a', [], False),
    (0x691ee0, 0x691ef0, 't', 1, 'a', [], False),
    (0x692970, 0x692980, 't', 1, 'a', [], False),
    (0x69fef0, 0x69ff10, 't', 1, 'a', [], False),
    (0x6a8cd0, 0x6a8ce0, 't', 1, 'a', [], True),
    (0x6a8ce0, 0x6a8cf0, 't', 1, 'a', [], True),
    (0x6b4a00, 0x6b4a10, 's', 3, 'a', [], True),
    (0x6b4a60, 0x6b4a70, 't', 0, 'a', [], False),
    (0x6b9c50, 0x6b9c70, 't', 0, 'a', [], False),
    (0x6b9c70, 0x6b9c90, 't', 1, 'a', [], False),
    (0x6b9cb0, 0x6b9cd0, 't', 1, 'a', [], False),
    (0x6b9cf0, 0x6b9d10, 't', 1, 'a', [], False),
    (0x6b9d10, 0x6b9d30, 't', 1, 'a', [], False),
    (0x6b9d30, 0x6b9d50, 't', 1, 'a', [], False),
    (0x6c47b0, 0x6c47c0, 't', 0, 'a', [], False),
    (0x6c4fb0, 0x6c4fd0, 't', 0, 'a', [], False),
    (0x6c4fd0, 0x6c4ff0, 't', 1, 'a', [], False),
    (0x6c5090, 0x6c50b0, 't', 1, 'a', [], False),
    (0x6c50b0, 0x6c50d0, 't', 1, 'a', [], False),
    (0x6c50d0, 0x6c50f0, 't', 1, 'a', [], False),
    (0x6c50f0, 0x6c5110, 't', 1, 'a', [], False),
    (0x6c5110, 0x6c5130, 't', 1, 'a', [], False),
    (0x6c5130, 0x6c5150, 't', 1, 'a', [], False),
    (0x6c5150, 0x6c5170, 't', 1, 'a', [], False),
    (0x6c5170, 0x6c5190, 't', 1, 'a', [], False),
    (0x6c51d0, 0x6c51f0, 't', 1, 'a', [], False),
    (0x6c5220, 0x6c5240, 't', 1, 'a', [], False),
    (0x6c6d30, 0x6c6d50, 't', 0, 'a', [], False),
    (0x6c9260, 0x6c9270, 't', 0, 'a', [], False),
    (0x6d2790, 0x6d27a0, 's', 5, 'a', [], True),
    (0x6d27a0, 0x6d27b0, 's', 1, 'a', [], True),
    (0x6d29e0, 0x6d29f0, 's', 4, 'a', [], True),
    (0x6d6c50, 0x6d6c70, 't', 0, 'a', [], False),
]
B = Bench([(a, e) for a, e, *_ in FUNCS])


def case(r):
    address, _end, conv, n, ret, globs, ptr = r.choice(FUNCS)
    regions = {RAM: r.bytes(0x400)}
    for g in globs:
        regions[g & ~0xf] = r.bytes(0x20)
    args = [r.choice([0, 1, 0xff, 0x7fffffff, 0xffffffff, r.getrandbits(32)]) for _ in range(n)]
    if ptr:
        args[0] = RAM + 0x200 + r.choice([0, 4, 0x20])
    if conv == 't':
        args = [RAM + 0x80] + args
    return Call(address, args, regions, conv=conv, ret=ret)


if __name__ == '__main__':
    B.main(case)
