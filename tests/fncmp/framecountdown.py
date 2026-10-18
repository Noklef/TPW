# The frame countdown (src/game/core/framecountdown: 0x4a9350) against the original: the counter 0x7cc150 negative, 0, 1 or more, the
# pending flag 0x7cc190 clear or set (any bits).
# usage: framecountdown.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Step)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

CALLS = {'Step': 0x4a9350}
B = Bench([(0x4a9350, 0x4a9375)])


def case(r):
    op = r.deal(OPS)
    regions = {0x7cc150: struct.pack('<i', r.choice([-3, -1, 0, 0, 1, 2, 5])) + bytes(0x3c) + struct.pack('<I', r.choice([0, 0, 1, 0xffff0000]))}
    return Call(CALLS[op], [], regions, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
