# The advisor's record arrays' element constructors (src/game/world/advisorrecords: 0x518e60, 0x518e80) against the originals: a
# record of random bytes.
# usage: advisorrecords.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Slot,Record)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call  # noqa: E402

CALLS = {'Slot': 0x518e60, 'Record': 0x518e80}
B = Bench([(0x518e60, 0x518e67), (0x518e80, 0x518e94)])
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = r.deal(OPS)
    return Call(CALLS[op], [RAM], {RAM: r.bytes(0x20)}, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
