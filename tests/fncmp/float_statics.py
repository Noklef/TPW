# The per-unit static initialisers of the two unused floats every translation unit gets from a shared header (1 and 1 / 2^23):
# GameClock, StdTree, Save, Animator, Instance, Highlight, SpriteScripts and CoasterCars (src/game/...: <Prefix>_InitUnusedOne /
# _InitUnusedEpsilon) and those of units unknown (src/game/core/floatconsts: FloatConsts_InitOne_<addr> / _InitEpsilon_<addr>)
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
         'SpriteScripts': (0x475fa0, 0x7b4a1c, 0x7b4a20), 'CoasterCars': (0x43bbe0, 0x7923a0, 0x7923a4),
         'FloatConsts407b20': (0x407b20, 0x78639c, 0x786aec),
         'FloatConsts409f60': (0x409f60, 0x78736c, 0x787a5c),
         'FloatConsts40d110': (0x40d110, 0x788394, 0x788a84),
         'FloatConsts4148d0': (0x4148d0, 0x78a3e0, 0x78a458),
         'FloatConsts438760': (0x438760, 0x792224, 0x792228),
         'FloatConsts43afd0': (0x43afd0, 0x792390, 0x792394),
         'FloatConsts43dbb0': (0x43dbb0, 0x7923b8, 0x7923d4),
         'FloatConsts45f950': (0x45f950, 0x7a3f1c, 0x7ae610),
         'FloatConsts470990': (0x470990, 0x7b4970, 0x7b4978),
         'FloatConsts4740e0': (0x4740e0, 0x7b4984, 0x7b498c),
         'FloatConsts475fe0': (0x475fe0, 0x7b4a24, 0x7b4a28),
         'FloatConsts476a30': (0x476a30, 0x7b4a2c, 0x7b4a30),
         'FloatConsts48c9a0': (0x48c9a0, 0x7c265c, 0x7c2664),
         'FloatConsts491fa0': (0x491fa0, 0x7ca0b8, 0x7ca0dc),
         }
B = Bench([r for one, _, _ in PAIRS.values() for r in ((one, one + 0x14), (one + 0x20, one + 0x33))])
OPS_ALL = [p + k for p in PAIRS for k in ('One', 'Epsilon')]
OPS = [k for k in os.environ.get('OPS', ','.join(OPS_ALL)).split(',') if k]


def case(r):
    op = r.deal(OPS)   # (a deck: each of the many initialisers gets its share of a short run)
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
