# The terrain texture tile cache's constructor (src/game/terrain/texcache: 0x5720d0) against the
# original: any bytes before.
# usage: texcache.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5720d0, 0x572100)])


def case(r):
    return Call(0x5720d0, [RAM], {RAM: r.bytes(0x10018)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
