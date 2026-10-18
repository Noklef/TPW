# The conversion facet's length slot (src/game/core/codecvt: 0x5d1440) against the original: from, to and the limit of 0, small, the
# signed limits, all ones or anything, so the span wraps and the limit falls on either side of it.
# usage: codecvt.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x5d1440, 0x5d1458)])


def case(r):
    v = lambda: r.choice([0, 1, 5, 100, 0x7fffffff, 0x80000000, 0xffffffff, r.getrandbits(32)])
    return Call(0x5d1440, [v(), v(), v(), v()], {}, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
