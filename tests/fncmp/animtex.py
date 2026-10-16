# The animated texture tables (src/game/gfx/animtex: 0x5906e0, 0x590730) against the originals:
# any tables, sources apart from them.
# usage: animtex.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Set,Clear)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

B = Bench([(0x5906e0, 0x590741)])
CALLS = {'Set': 0x5906e0, 'Clear': 0x590730}


def case(r):
    op = r.choice(OPS)
    regions = {0x8c4b70: r.bytes(0x1840)}
    if op == 'Clear':
        return Call(0x590730, [], regions)
    regions[RAM] = r.bytes(0x1820)
    return Call(0x5906e0, [RAM, RAM + 0x1800], regions)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
