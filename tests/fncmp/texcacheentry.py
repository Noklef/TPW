# The texture cache entry constructor (src/game/gfx/texcache: 0x590fd0) against the original: any
# bytes before.
# usage: texcacheentry.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x590fd0, 0x590fe7)])


def case(r):
    return Call(0x590fd0, [RAM], {RAM: r.bytes(0x20)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
