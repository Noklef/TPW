# The integer sine table's lookups (src/game/math/intsin: 0x5428a0, 0x5428c0) against the
# originals: any angle, about the wrap at 512, over any table.
# usage: intsin.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x5428a0, 0x5428b4), (0x5428c0, 0x5428d9)])


def case(r):
    angle = r.choice([0, 0x7f, 0x80, 0x17f, 0x180, 0x1ff, 0x200, r.randrange(0x400), r.getrandbits(32)])
    return Call(r.choice([0x5428a0, 0x5428c0]), [angle], {0x877358: r.bytes(0x800)}, ret='a')


if __name__ == '__main__':
    B.main(case)
