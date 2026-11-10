# The cseg angle routine (src/game/math/intangle: 0x6e7074) against the original: vectors of every quadrant and steepness, small and
# large (past 2^24, where the shift by 8 wraps), on the axes and diagonals, and 0x80000000 in either (which stays negative; with a zero
# beside it the division faults in both).
# usage: intangle.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x6e7074, 0x6e71b4)])


def value(r):
    k = r.randrange(8)
    if k < 3:
        return r.randrange(-300, 301)
    if k < 5:
        return r.randrange(-(1 << 20), 1 << 20)
    if k == 5:
        return r.randrange(-(1 << 31), 1 << 31)
    return r.choice([0, 1, -1, 0x7fffffff, -0x80000000, 1 << 24, -(1 << 24)])


def case(r):
    dx, dy = value(r), value(r)
    k = r.randrange(10)
    if k == 0:
        dy = r.choice([dx, -dx])          # diagonals: the entry at 256
    elif k == 1 and (dx, dy) != (0, 0):
        dx, dy = r.choice([(dx, 0), (0, dy)])
    if r.randrange(40) == 0:
        dx, dy = r.choice([(-0x80000000, dy | 1), (dx | 1, -0x80000000), (-0x80000000, -0x80000000)])
    return Call(0x6e7074, [dx & 0xffffffff, dy & 0xffffffff], {}, ret='a')


if __name__ == '__main__':
    B.main(case)
