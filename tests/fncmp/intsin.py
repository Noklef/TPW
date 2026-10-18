# The integer sine table (src/game/math/intsin: 0x5428a0, 0x5428c0, 0x5442e0) and the particle pools' reset that builds it
# (src/game/render/particles: 0x5ed580) against the originals. Sin / Cos: any angle, about the wrap at 512, over any table. Build / Reset:
# the table (and the pools' block, from 0xf86900) random with guard bytes each side.
# usage: intsin.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Sin,Cos,Build,Reset)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

CALLS = {'Sin': 0x5428a0, 'Cos': 0x5428c0, 'Build': 0x5442e0, 'Reset': 0x5ed580}
B = Bench([(0x5428a0, 0x5428b4), (0x5428c0, 0x5428d9), (0x5442e0, 0x544321), (0x5ed580, 0x5ed596)])
OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


def case(r):
    op = r.deal(OPS)
    if op in ('Sin', 'Cos'):
        angle = r.choice([0, 0x7f, 0x80, 0x17f, 0x180, 0x1ff, 0x200, r.randrange(0x400), r.getrandbits(32)])
        return Call(CALLS[op], [angle], {0x877358: r.bytes(0x800)}, ret='a')
    regions = {0x877350: r.bytes(0x810)}
    if op == 'Reset':
        regions[0xf868f0] = r.bytes(0xf0)
    return Call(CALLS[op], [], regions)


if __name__ == '__main__':
    B.main(case)
