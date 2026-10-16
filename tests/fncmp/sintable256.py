# The renderer's 256-entry sine table (src/game/math/sintable: 0x5913f0) against the original.
# usage: sintable256.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x5913f0, 0x591428)])


def case(r):
    return Call(0x5913f0, [], {0x8c63d0: r.bytes(0x410)})


if __name__ == '__main__':
    B.main(case)
