# Staff_MarkPatrolCells (src/game/world/staff, 0x4cd340) against the original: World_FirstThing and World_NextThing real over a list of things (none now and then), each of a kind among 4-8 and others, its patrol region none (1, 0x4000), a small one, the whole map, reversed or with a corner 0. The map's cells a constant fill, so
# the clearing and the marks show; a low case count: its 128 x 128 loop is slow here.
# usage: staffpatrolcells.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x4cd340, 0x4cd4d5), (0x4cd4e0, 0x4cd4f7), (0x516130, 0x516152), (0x516160, 0x5161a5), (0x50b350, 0x50b35d), (0x506e70, 0x506e90), (0x506eb0, 0x506ee0), (0x5da3c0, 0x5da3c1)])
THINGS, WORLD = RAM, RAM + 0x10000
SIZE = 0x2d8 + 0x4001 * 0x44


def region(r):
    x0, z0 = r.randrange(0x80), r.randrange(0x80)
    x1, z1 = min(0x7f, x0 + r.randrange(0, 6)), min(0x7f, z0 + r.randrange(0, 6))
    return r.choice([(1, 0x4000), ((z0 << 7) + x0 + 1, (z1 << 7) + x1 + 1), ((z0 << 7) + x0 + 1, (z1 << 7) + x1 + 1), (r.randrange(0, 0x4001), r.randrange(1, 0x4001)), (1, 0x3fff), (2, 0x4000), (0, r.randrange(1, 0x4001))])


def case(r):
    regions = {}
    kind = r.deal([4, 5, 6, 7, 8])
    kind = r.choice([kind] * 8 + [3, 9, 0x104, 0])
    handles = r.sample(range(1, 0x400), r.choice([0, 1, 2, 3, 5]))
    regions[0x7cf56c] = struct.pack('<I', 0x7cfb90 + 20 * handles[0] if handles else 0)
    for k, h in enumerate(handles):
        thing = bytearray(r.bytes(0x210))
        put(thing, 0, 'H', h)
        put(thing, 2, 'B', r.choice([kind & 0xff, kind & 0xff, 4, 5, 6, 7, 8, 3, 9, 1]))
        put(thing, 0x20a, 'H', 0)
        bottom, top = region(r)
        put(thing, 0x20a, 'H', bottom)
        put(thing, 0x20c, 'H', top)
        at = THINGS + 0x210 * k
        regions[at] = thing
        regions[0x7cfb90 + 20 * h] = struct.pack('<3I', at, h | r.getrandbits(16) << 16, 0x7cfb90 + 20 * handles[k + 1] if k + 1 < len(handles) else 0)
    regions[WORLD] = bytes([r.choice([0x5a, 0xa5, 0xff])]) * SIZE
    regions[0x80239c] = struct.pack('<I', WORLD)
    return Call(0x4cd340, [kind], regions, conv='s', ret='v', steps=20_000_000)


if __name__ == '__main__':
    B.main(case)
