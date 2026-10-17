# The save game's field announcer (src/game/core/savegame: 0x416e10) against the original: random sizes, names and load flags; it returns 0 and
# pops its three arguments.
# usage: savegame.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x416e10, 0x416e15)])


def case(r):
    return Call(0x416e10, [r.getrandbits(32), r.getrandbits(32), r.choice([0, 1])], {}, conv='s', ret='a')


if __name__ == '__main__':
    B.main(case)
