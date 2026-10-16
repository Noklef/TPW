# The texture manager's load flags setter (src/game/gfx/texman: 0x575b80) against the original: any
# flags over any old ones.
# usage: texman.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x575b80, 0x575b8d)])


def case(r):
    regions = {RAM + 0x10000: r.bytes(0x10)}
    return Call(0x575b80, [RAM, r.choice([0, 2, 4, 8, r.getrandbits(32)])], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
