# The world's random numbers (src/game/world/random: 0x516330 World_Rand, 0x516370 World_SetSeed, thiscall on the world) against the originals:
# seeds random, 0, and the values whose rotated next state is negative or 0x80000000 (NEG keeps it).
# usage: worldrandom.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Rand,SetSeed)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call  # noqa: E402

B = Bench([(0x516330, 0x51637d)])
WORLD = 0x31000000
SEED = WORLD + 0x1da708
CALLS = {'Rand': 0x516330, 'SetSeed': 0x516370}


def before(x):
    """The seed whose next state, rotated right by 13, is x."""
    s = ((x << 13) | (x >> 19)) & 0xffffffff
    return ((s - 0x3c6ef35f) * pow(0x19660d, -1, 1 << 32)) & 0xffffffff


def case(r):
    op = r.deal(OPS)
    seed = r.choice([r.getrandbits(32), 0, 1, 0xffffffff, before(0x80000000), before(0), before(r.getrandbits(32) | 0x80000000)])
    regions = {SEED: struct.pack('<I', seed)}
    if op == 'Rand':
        return Call(0x516330, [WORLD], regions, conv='t', ret='a')
    return Call(0x516370, [WORLD, r.getrandbits(32)], regions, conv='t', ret='v')   # (EAX: the seed, only as the store's temporary)


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
