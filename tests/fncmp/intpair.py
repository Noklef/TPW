# The int pair's copy and scale (src/game/math/intpair: 0x4d99c0, 0x4d97b0) against the
# originals: apart, the same, and overlapping by one int either way; scales of any value.
# usage: intpair.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x4d99c0, 0x4d99d3), (0x4d97b0, 0x4d97d9)])


def case(r):
    pair = RAM + 0x100
    other = r.choice([RAM + 0x200, pair, pair + 4, pair - 4])
    if r.random() < 0.5:
        return Call(0x4d99c0, [pair, other], {RAM: r.bytes(0x400)}, conv='t', ret='a')
    scale = r.choice([0, 1, 2, -1 & 0xffffffff, r.randrange(100), r.getrandbits(32)])
    return Call(0x4d97b0, [pair, other, scale], {RAM: r.bytes(0x400)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
