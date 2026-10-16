# The nearest-thing searches (src/game/world/staff: 0x500f00, 0x5099c0) against the originals: lists of up to six things chained through the word at +0x210
# from the head handles at +0x1da740 / +0x1da744 of the game object, the things at map columns and rows (bytes +5 and +7) around the searcher's.
# usage: nearest.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Ent,Guard)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, THING, park  # noqa: E402

CALLS = {'Ent': (0x500f00, 't', 0x120, 0x1da740, 1), 'Guard': (0x5099c0, 't', 0x120, 0x1da744, 2)}
B = Bench([(a, a + n) for (a, _c, n, _o, _k) in CALLS.values()] + [(0x4d6330, 0x4d6360), (0x5da3c0, 0x5da3c1)])
GAME, OUT, SELF = RAM + 0x20000, RAM + 0x1000, RAM + 0x800


def case(r):
    name = r.choice(OPS)
    address, conv, _n, field, nargs = CALLS[name]
    regions, handles, _a = park(r, count=r.choice([0, 1, 2, 3, 4, 6]), defs=False)
    pool = regions[POOL]
    for i, h in enumerate(handles):
        struct.pack_into('<H', pool, THING * i + 0x210, handles[i + 1] if i + 1 < len(handles) and r.random() < 0.8 else 0)
        pool[THING * i + 5] = r.choice([0, 1, 5, 20, 127, 128, 255, r.getrandbits(8)])
        pool[THING * i + 7] = r.choice([0, 1, 5, 20, 127, 128, 255, r.getrandbits(8)])
        struct.pack_into('<I', pool, THING * i + 0x19c, r.choice([0, 0, 1, 1, 2, 5]))
    me = bytearray(r.bytes(0x40))
    me[5], me[7] = r.choice([0, 1, 5, 20, 127, 255]), r.choice([0, 1, 5, 20, 127, 255])
    game = bytearray(r.bytes(0x20))
    regions[0x80239c] = struct.pack('<I', GAME - field)
    regions[GAME] = struct.pack('<H', r.choice(handles + [0]) if handles else 0) + bytes(2)
    regions[SELF] = me
    regions[OUT] = r.bytes(4)
    return Call(address, [SELF, OUT] + ([r.getrandbits(32)] if nargs == 2 else []), regions, conv=conv, ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
