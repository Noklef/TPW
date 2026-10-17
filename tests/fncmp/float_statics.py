# The per-unit static initialisers of the two unused floats every translation unit gets from a shared header (1 and 1 / 2^23):
# GameClock, StdTree, Save, Animator, Instance, Highlight and SpriteScripts (src/game/...: <Prefix>_InitUnusedOne / _InitUnusedEpsilon)
# against the originals, each over random bytes around its global.
# usage: float_statics.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

# prefix: (address of the "one" initialiser, its global, the epsilon one's global); the epsilon initialiser is at +0x20
PAIRS = {'GameClock': (0x4033e0, 0x785a04, 0x785ddc), 'StdTree': (0x410be0, 0x7899b4, 0x78a0bc),
         'Save': (0x416e20, 0x78aa2c, 0x78b11c), 'Animator': (0x4255b0, 0x78ed8c, 0x78f47c),
         'Instance': (0x455bf0, 0x7a0174, 0x7a086c), 'Highlight': (0x4698f0, 0x7afe1c, 0x7b0464),
         'SpriteScripts': (0x475fa0, 0x7b4a1c, 0x7b4a20)}
B = Bench([r for one, _, _ in PAIRS.values() for r in ((one, one + 0x14), (one + 0x20, one + 0x33))])
OPS_ALL = [p + k for p in PAIRS for k in ('One', 'Epsilon')]
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


def case(r):
    op = r.choice(OPS)
    eps = op.endswith('Epsilon')
    one, g1, g2 = PAIRS[op[:-len('Epsilon')] if eps else op[:-len('One')]]
    g = g2 if eps else g1
    regions = {g - 8: bytearray(r.bytes(0x14))}
    if (g2 if not eps else g1) - g not in range(-8, 0xc):  # the other global too, unless it is in the same window
        other = g1 if eps else g2
        regions[other - 8] = bytearray(r.bytes(0x14))
    return Call(one + 0x20 if eps else one, [], regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
