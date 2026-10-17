# The nearest-thing searches of src/game/world/thingscan (0x506910 rest areas, 0x501020 flag 0x80 within distance 2) against the originals: the game's chain of things (head word
# at +0x1da746 of [0x80239c], each thing's next at +0xc) with flags and cells close together, ridelist.py's ride records for the script test, the thing to measure from, and an
# output word.
# usage: thing_nearest.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Rest,Flag80)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import ridelist  # noqa: E402
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import POOL, SLOTS, THING, park  # noqa: E402

B = Bench([(0x501020, 0x501170), (0x506910, 0x506a50), (0x55a070, 0x55a0c0), (0x55a390, 0x55a3f0), (0x5da3c0, 0x5da3c1)])
GAME, ME, OUT = RAM + 0x40000, RAM + 0x41000, RAM + 0x42000


def case(r):
    name = r.choice(OPS)
    if name == 'Flag80':
        ridelist.OPS = ['GameE']
        c = ridelist.case(r)
        regions = dict(c.regions)
        pool = bytearray(regions[POOL])
    else:
        regions, _handles, _things = park(r, defs=False)
        pool = bytearray(regions[POOL])
    slots = regions[SLOTS]
    n = sum(1 for k in range(1, 12) if POOL <= struct.unpack_from('<I', slots, 20 * k)[0] < POOL + 6 * THING)
    handles = [struct.unpack_from('<H', pool, THING * i)[0] for i in range(n)]
    for i in range(n):
        put(pool, THING * i + 0xc, 'H', handles[i + 1] if i + 1 < n else 0)
        pool[THING * i + 0x32] = r.choice([0, 2, 0x80, 0x82, 0xff, 0x7d, r.getrandbits(8)])
        pool[THING * i + 5], pool[THING * i + 7] = r.randrange(0, 5), r.randrange(0, 4)
    regions[POOL] = pool
    regions[GAME] = struct.pack('<H', handles[0] if n else 0)
    regions[0x80239c] = struct.pack('<I', GAME - 0x1da746)
    me = bytearray(r.bytes(0x40))
    me[5], me[7] = r.randrange(0, 5), r.randrange(0, 4)
    regions[ME] = me
    regions[OUT] = r.bytes(4)
    return Call(0x501020 if name == 'Flag80' else 0x506910, [ME, OUT], regions, conv='t', ret='a')


OPS = [k for k in os.environ.get('OPS', 'Rest,Flag80').split(',') if k]

if __name__ == '__main__':
    B.main(case)
