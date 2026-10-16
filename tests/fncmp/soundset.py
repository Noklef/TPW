# The sound settings' setters (src/game/core/soundset: 0x51bb60-0x51bc20) against the originals.
# usage: soundset.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x51bb60, 0x51bc34)])


def case(r):
    fn = r.choice([0x51bb60, 0x51bb80, 0x51bba0, 0x51bbc0, 0x51bbe0, 0x51bc00, 0x51bc20])
    return Call(fn, [r.choice([0, 1, 100, r.getrandbits(32)])], {0x803ab0: r.bytes(0x40)})


if __name__ == '__main__':
    B.main(case)
