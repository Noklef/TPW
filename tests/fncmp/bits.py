# The colour mask bit counts (src/game/math/bits: 0x5911d0, 0x5911f0) against the originals: masks
# of runs of bits, single bits, zero and anything.
# usage: bits.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Count,Shift)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x5911d0, 0x5911e5), (0x5911f0, 0x591209)])
CALLS = {'Count': 0x5911d0, 'Shift': 0x5911f0}


def case(r):
    op = r.choice(OPS)
    width, shift = r.randrange(33), r.randrange(32)
    run = (((1 << width) - 1) << shift) & 0xffffffff
    mask = r.choice([run, 0, 0xffffffff, 0x80000000, 1 << shift, r.getrandbits(32)])
    return Call(CALLS[op], [mask], {}, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
