# The help background (src/game/frontend/helpbg: 0x4922d0) against the original: any object bytes. (Its HelpBg_Refresh, 0x492320, is
# benched with the virtual call forwarders in vcall_forwards.py.)
# usage: helpbg.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM  # noqa: E402

CALLS = {'Clear': 0x4922d0}
B = Bench([(0x4922d0, 0x4922e0)])


def case(r):
    op = r.deal(OPS)
    return Call(CALLS[op], [RAM + 0x20], {RAM: r.bytes(0x60)}, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
