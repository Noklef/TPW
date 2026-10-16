# The nav's position and velocity accessors (src/game/world/nav: 0x5100e0, 0x510160) against the
# originals: outs apart, the same, or over the position.
# usage: nav.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5100e0, 0x5100ff), (0x510160, 0x510175)])
NAV = RAM + 0x100


def case(r):
    regions = {RAM: r.bytes(0x200)}
    if r.random() < 0.5:
        return Call(0x5100e0, [NAV] + [r.getrandbits(32) for _ in range(4)], regions, conv='t')
    x = RAM + r.choice([0x10, 0x108, 0x10c])
    return Call(0x510160, [NAV, x, r.choice([x, RAM + 0x20, RAM + 0x108])], regions, conv='t')


if __name__ == '__main__':
    B.main(case)
