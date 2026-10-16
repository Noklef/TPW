# The shared trivial constructor (src/game/core/regkey: 0x581da0) against the original.
# usage: ptrctor.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x581da0, 0x581da7)])


def case(r):
    return Call(0x581da0, [RAM + 4], {RAM: r.bytes(0xc)}, conv='t')


if __name__ == '__main__':
    B.main(case)
