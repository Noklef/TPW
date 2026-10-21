# The eight unit steps' atexit entries (src/game/math/intpair: 0x4d9a90, 0x4d9ae0, 0x4d9b30, 0x4d9b70, 0x4d9bc0, 0x4d9c10, 0x4d9c60,
# 0x4d9ca0) against the originals: the flags byte 0x7cdbe0 (and its neighbours, the steps' pairs below it) any, the entry's bit set or clear.
# usage: intpairexits.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=North,NorthEast,...)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

CALLS = {'North': 0x4d9a90, 'NorthEast': 0x4d9ae0, 'East': 0x4d9b30, 'SouthEast': 0x4d9b70, 'South': 0x4d9bc0, 'SouthWest': 0x4d9c10,
         'West': 0x4d9c60, 'NorthWest': 0x4d9ca0}
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]
B = Bench([(a, a + 0x15) for a in CALLS.values()])


def case(r):
    op = r.deal(OPS)
    bit = 1 << list(CALLS).index(op)
    flags = r.choice([0, 0xff, bit, 0xff ^ bit, r.getrandbits(8)])
    return Call(CALLS[op], [], {0x7cdba0: r.bytes(0x40) + bytes([flags]) + r.bytes(3)}, ret='v')


if __name__ == '__main__':
    B.main(case)
