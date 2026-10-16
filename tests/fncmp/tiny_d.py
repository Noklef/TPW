# The tiny functions of src/game/core/tiny_d (written by tools/tinygen.py) against the originals: any
# object bytes, any global values, any arguments.
# usage: tiny_d.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

# (address, end, convention, stack arguments, returns, globals touched, arguments used as pointers)
FUNCS = [
    (0x47f910, 0x47f940, 't', 1, 'a', [], (0,)),
    (0x48c9e0, 0x48ca00, 't', 0, 'a', [8136280], ()),
    (0x4e0210, 0x4e0220, 't', 0, 'a', [], ()),
    (0x4e0de0, 0x4e0df0, 't', 1, 'w', [], ()),
    (0x4f0150, 0x4f0170, 't', 1, 'a', [], (0,)),
    (0x4f6010, 0x4f6020, 'c', 0, 'b', [], ()),
    (0x509ae0, 0x509af0, 't', 1, 'w', [], ()),
    (0x50b080, 0x50b090, 't', 1, 'w', [], ()),
    (0x52c2a0, 0x52c2b0, 'c', 0, 'v', [8490068], ()),
    (0x52c6c0, 0x52c6f0, 'c', 4, 'a', [8488676, 8488680, 8489996, 8489992], (0, 1, 2, 3)),
    (0x52fbd0, 0x52fbe0, 'c', 0, 'v', [8522380], ()),
    (0x564730, 0x564750, 't', 2, 'a', [], ()),
    (0x56b620, 0x56b630, 's', 1, 'a', [], ()),
    (0x5ce530, 0x5ce540, 't', 1, 'a', [], (0,)),
    (0x5cec50, 0x5cec60, 't', 2, 'a', [], ()),
    (0x5d0e00, 0x5d0e30, 't', 6, 'a', [], ()),
    (0x5d1400, 0x5d1410, 'c', 0, 'b', [], ()),
    (0x5d1420, 0x5d1440, 's', 7, 'a', [], (3, 6)),
    (0x5d29b0, 0x5d29d0, 't', 0, 'a', [16263148, 16263152], ()),
    (0x5d5f70, 0x5d5f80, 't', 0, 'a', [], ()),
    (0x5e1bb0, 0x5e1bd0, 't', 3, 'a', [], ()),
    (0x5ebe30, 0x5ebe50, 't', 3, 'a', [], ()),
    (0x5ebef0, 0x5ebf10, 't', 0, 'a', [], ()),
    (0x5eda40, 0x5eda70, 'c', 2, 'a', [16279944, 16279948, 16279952, 16279956], ()),
    (0x64a450, 0x64a480, 'c', 4, 'a', [7839896, 7839900], (0, 1, 2, 3)),
    (0x64dd60, 0x64ddec, 'c', 1, 'a', [], (0,)),
    (0x657d30, 0x657d6c, 't', 0, 'a', [], ()),
    (0x6590c3, 0x6590d0, 'c', 1, 'a', [16426452], ()),
    (0x6590ea, 0x6590f7, 'c', 1, 'a', [16426492], ()),
    (0x6593ff, 0x659409, 'c', 0, 'a', [7849096], ()),
    (0x6595a8, 0x6595b3, 'c', 0, 'w', [16426428], ()),
    (0x659770, 0x659790, 't', 0, 'a', [], ()),
    (0x659ce0, 0x659d00, 't', 0, 'a', [], ()),
    (0x65a9d0, 0x65a9f0, 't', 0, 'a', [], ()),
    (0x65aa80, 0x65aa90, 't', 1, 'a', [], ()),
    (0x65b850, 0x65b864, 't', 0, 'a', [], ()),
    (0x65bc70, 0x65bc90, 't', 0, 'a', [], ()),
    (0x65d547, 0x65d55a, 't', 1, 'a', [], ()),
    (0x65d55a, 0x65d56b, 't', 0, 'a', [], ()),
    (0x65d56b, 0x65d57c, 't', 0, 'a', [], ()),
    (0x65d57c, 0x65d58d, 't', 0, 'a', [], ()),
    (0x65d7bc, 0x65d7d0, 't', 0, 'a', [], ()),
    (0x6617d0, 0x6617f0, 't', 1, 'a', [], ()),
    (0x6617f0, 0x661810, 't', 0, 'a', [], ()),
    (0x661830, 0x661850, 't', 1, 'a', [], ()),
    (0x661850, 0x661870, 't', 1, 'a', [], ()),
    (0x661870, 0x6618a6, 't', 0, 'a', [], ()),
    (0x6618a6, 0x6618ba, 't', 0, 'a', [], ()),
    (0x6618e6, 0x6618f8, 't', 0, 'a', [], ()),
    (0x6618f8, 0x66190a, 't', 0, 'a', [], ()),
    (0x661c0b, 0x661c2b, 't', 0, 'a', [], ()),
    (0x661f9b, 0x661faa, 'c', 1, 'a', [], (0,)),
    (0x662086, 0x662097, 't', 0, 'a', [], ()),
    (0x662097, 0x6620ac, 't', 0, 'a', [], ()),
    (0x6620cc, 0x6620e2, 't', 1, 'a', [], ()),
    (0x662411, 0x662420, 'c', 0, 'v', [16426552], ()),
    (0x667801, 0x66781a, 't', 1, 'a', [], ()),
    (0x66781a, 0x667833, 't', 1, 'a', [], ()),
    (0x668d84, 0x668d98, 't', 0, 'a', [], ()),
    (0x6693d0, 0x6693f0, 't', 1, 'a', [], ()),
    (0x6693f0, 0x669410, 't', 1, 'a', [], ()),
    (0x669700, 0x669735, 't', 0, 'a', [], ()),
    (0x669c30, 0x669c41, 't', 0, 'a', [], ()),
    (0x669c41, 0x669c62, 't', 0, 'a', [], ()),
    (0x669f80, 0x669f94, 't', 0, 'a', [], ()),
    (0x669f94, 0x669fa8, 't', 0, 'a', [], ()),
    (0x669fa8, 0x669fee, 't', 0, 'a', [], ()),
    (0x669fee, 0x66a002, 't', 0, 'a', [], ()),
    (0x66a002, 0x66a016, 't', 0, 'a', [], ()),
    (0x66a281, 0x66a295, 't', 0, 'a', [], ()),
    (0x66a6c7, 0x66a6db, 't', 0, 'a', [], ()),
    (0x66b469, 0x66b47d, 't', 0, 'a', [], ()),
    (0x66b53e, 0x66b557, 't', 1, 'a', [], ()),
    (0x66bff7, 0x66c00b, 't', 0, 'a', [], ()),
    (0x66c922, 0x66c940, 't', 0, 'a', [], ()),
    (0x66cc7c, 0x66cc90, 't', 0, 'a', [], ()),
    (0x66d950, 0x66d964, 't', 0, 'a', [], ()),
    (0x66dad2, 0x66daf0, 't', 0, 'a', [], ()),
    (0x67a610, 0x67a650, 'c', 0, 'a', [7859364, 7859360, 7859368, 7859372, 7859376, 7859380], ()),
    (0x682440, 0x682460, 'c', 1, 'a', [], (0,)),
    (0x68bda0, 0x68bdb0, 'c', 1, 'a', [], (0,)),
    (0x6a65e0, 0x6a65f0, 'c', 0, 'b', [], ()),
    (0x6b52e0, 0x6b5310, 't', 6, 'a', [], ()),
    (0x6b5310, 0x6b5340, 't', 6, 'a', [], ()),
    (0x6b5430, 0x6b5450, 't', 2, 'a', [], ()),
    (0x6bb220, 0x6bb240, 't', 3, 'a', [], ()),
    (0x6c0200, 0x6c0210, 's', 2, 'a', [], (1,)),
    (0x6c3900, 0x6c3910, 'c', 1, 'a', [], ()),
    (0x6c54c0, 0x6c54e0, 't', 2, 'a', [], (0, 1)),
    (0x6c58d0, 0x6c58f0, 't', 2, 'a', [], ()),
    (0x6c6a00, 0x6c6a40, 't', 2, 'a', [], (0,)),
    (0x6d1a50, 0x6d1a80, 't', 0, 'a', [], ()),
    (0x6d2780, 0x6d2790, 's', 2, 'a', [], (1,)),
    (0x6d3b90, 0x6d3bb0, 't', 0, 'a', [], ()),
]
B = Bench([(a, e) for a, e, *_ in FUNCS])


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
