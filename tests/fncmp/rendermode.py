# The render mode word's flag test (src/game/render/mode: 0x54dcc0) against the original: the screen flag
# 0x78d968 set or not, the option 0x785874 0, 1 or other, any mode word with bit 11 set or clear.
# usage: rendermode.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, put  # noqa: E402

B = Bench([(0x54dcc0, 0x54dcf0)])


def case(r):
    regions = {0x78d960: put(bytearray(r.bytes(0x10)), 8, 'I', r.choice([0, 0, 1])), 0x785870: put(bytearray(r.bytes(0x10)), 4, 'I', r.choice([0, 1, 2, 5, 0x100])),
               0x79d860: put(bytearray(r.bytes(0x10)), 0xc, 'I', r.choice([0, 0x800, 0x7ff, 0xffffffff, r.getrandbits(32)]))}
    return Call(0x54dcc0, [], regions, ret='b')


if __name__ == '__main__':
    B.main(case)
