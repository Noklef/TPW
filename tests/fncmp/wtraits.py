# The 16-bit character move (src/game/core/wtraits: 0x479150) against the original: overlapping
# either way, odd-aligned, apart, empty.
# usage: wtraits.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x479150, 0x47919d)])


def case(r):
    n = r.choice([0, 1, 2, r.randrange(3, 40)])
    src = RAM + 0x100 + r.randrange(0x40)
    dst = r.choice([src + r.randrange(-80, 81), RAM + 0x800 + r.randrange(0x40), src])
    return Call(0x479150, [dst, src, n], {RAM: r.bytes(0x1000)}, ret='a')


if __name__ == '__main__':
    B.main(case)
