# The small functions of src/game/core/misc against the originals: any object bytes, any positions.
# usage: misc.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear0,Pixel,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x5836b0, 0x5836c2), (0x56b620, 0x56b628), (0x633dc0, 0x633e00), (0x6486e0, 0x648720), (0x5d36c0, 0x5d36d0),
           (0x633120, 0x63313a), (0x6338b0, 0x6338c0), (0x5f6a20, 0x5f6a60), (0x576540, 0x5765b8)])
CALLS = ['Ctor3ca0', 'MaxW', 'Ctor3c18']


def case(r):
    op = r.choice(OPS)
    obj = bytearray(r.bytes(0x400))
    regions = {RAM: obj}
    if op == 'Ctor3ca0':
        return Call(0x633dc0, [RAM], regions, conv='t', ret='a')
    if op == 'MaxW':
        n = r.choice([0, 1, 3, 6, -1])
        for i in range(6):
            put(obj, 0x20 * i + 0x18, 'i', r.choice([-1, 0, 1, 5]))
            put(obj, 0x20 * i + 0x1c, 'i', r.choice([-3, 0, 1, 5, 5, 9, 0x7fffffff]))
        return Call(0x6486e0, [RAM, n & 0xffffffff], regions, ret='a')
    if op == 'Ctor3c18':
        return Call(0x633120, [RAM], regions, conv='t', ret='a')
    raise ValueError(op)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
