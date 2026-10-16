# The renderer's light list (src/game/render/lightlist: 0x584b10, 0x584e30, 0x584e50) against the
# originals: counts and cursors around 0 to 4, negative and wild.
# usage: lightlist.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Clear,First,Next)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x584b10, 0x584b1f), (0x584e30, 0x584e71)])
CALLS = {'Clear': 0x584b10, 'First': 0x584e30, 'Next': 0x584e50}


def case(r):
    op = r.choice(OPS)
    mem = bytearray(r.bytes(0x1d0))
    small = [-1, 0, 1, 2, 3, 4, 5]
    put(mem, 0x1c0, 'I', r.choice(small + [r.getrandbits(32)]) & 0xffffffff)
    put(mem, 0x1c4, 'I', r.choice(small + [0x7fffffff, r.getrandbits(32)]) & 0xffffffff)
    return Call(CALLS[op], [RAM], {RAM: mem}, conv='t', ret='v' if op == 'Clear' else 'a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
