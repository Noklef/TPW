# The texture cache entry constructor and the cache's defaults (src/game/gfx/texcache: 0x590fd0,
# 0x58d0f0) against the originals: any bytes before.
# usage: texcacheentry.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x58d0f0, 0x58d1d0), (0x590fd0, 0x590fe7)])


def case(r):
    if r.random() < 0.5:
        return Call(0x58d0f0, [RAM], {RAM: r.bytes(0x1940)}, conv='t', ret='a')
    return Call(0x590fd0, [RAM], {RAM: r.bytes(0x20)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
