# The walk set-up between two cells (src/game/world/linestep: 0x511350) against the original:
# positions with any bytes in the column and row, equal ones and extremes included.
# usage: linestep.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x511350, 0x511412)])


def case(r):
    pos = lambda: r.choice([0, 0xffff, 0x0101, 0x8000, r.getrandbits(16)]) | r.choice([0, r.getrandbits(16) << 16])
    a = pos()
    b = a if r.random() < 0.1 else pos()
    k = r.random()
    if k < 0.25:  # the same column
        b = (b & ~0xff) | (a & 0xff)
    elif k < 0.5:  # the same row
        b = (b & ~0xff00) | (a & 0xff00)
    return Call(0x511350, [a, b], {0x7cf390: r.bytes(0x40)})


if __name__ == '__main__':
    B.main(case)
