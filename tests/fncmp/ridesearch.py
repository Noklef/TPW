# The track ride searches of src/game/world/ridesearch (0x4cc4b0, 0x4cc5d0, 0x4cc6f0) against the originals: a park of things (thingenv.py) whose objects have
# definitions of kinds 0 to 3, and track rides (trackrides.py: the module loaded, the pieces of kinds 8 to 14) behind their handles; the best ride's handle ends up in
# the out word.
# usage: ridesearch.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Name,...)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thingenv import ITEM, ITEMS, NITEMS, POOL, THING, park  # noqa: E402

# name: (address, size, with a second argument)
CALLS = {'Kind11': (0x4cc4b0, 0x120, False), 'Kinds': (0x4cc5d0, 0x120, True), 'Kind': (0x4cc6f0, 0x120, True)}
B = Bench([(a, a + n) for (a, n, _t) in CALLS.values()] + [(0x516130, 0x516160), (0x516160, 0x5161b0), (0x412e90, 0x412eea), (0x5da3c0, 0x5da3c1), (0x4dd4e0, 0x4dd500),
          (0x4e2b80, 0x4e2c10), (0x545310, 0x545450), (0x50b350, 0x50b360)])
OUT, RIDES, PIECES = RAM + 0x1000, RAM + 0x40000, RAM + 0x60000


def case(r):
    name = r.choice(OPS)
    address, _n, two = CALLS[name]
    regions, handles, addrs = park(r, count=r.choice([1, 2, 3, 4, 6]), defs=True)
    items = bytearray(regions[ITEMS])
    for i in range(NITEMS):
        put(items, i * ITEM + 0x9c, 'I', r.choice([1, 1, 2, 3, 0]))
        put(items, i * ITEM + 0x4ac, 'I', r.choice([0, 0, 0, 3]))
    regions[ITEMS] = items
    things = bytearray(regions[POOL])
    rides = bytearray(r.bytes(0x10 * 0xd0))
    pieces = bytearray(r.bytes(0x40 * 8 * len(addrs)))
    for i in range(len(addrs)):
        serial = r.choice([0, 1, r.getrandbits(16)])
        put(rides, i * 0xd0, 'I', serial)
        n = r.choice([0, 1, 2, 3, 4, 6])
        for k in range(n):
            at = 0x40 * (8 * i + k)
            put(pieces, at + 4, 'H', r.choice([8, 9, 10, 11, 11, 12, 13, 14]))
            put(pieces, at + 8, 'I', r.choice([0, 0, 1]))
            put(pieces, at + 0x24, 'I', PIECES + 0x40 * (8 * i + k + 1) if k + 1 < n else 0)
        put(rides, i * 0xd0 + 0xbc, 'I', PIECES + 0x40 * 8 * i if n else 0)
        put(things, THING * i + 0x28, 'I', i | (serial << 8) & 0xffffffff)
        if r.random() < 0.3:  # a thing that cannot be measured: skip nothing, the handle is good
            pass
    regions[POOL] = things
    regions.update({OUT: bytes(8), RIDES: rides, PIECES: pieces, 0x877b58: struct.pack('<IIII', 0x4a454647, 0, RIDES, 0) + bytes(0x30)})
    args = [OUT]
    if two:
        args.append(r.choice([0, 0, 1, 2, 3, r.randrange(NITEMS), r.getrandbits(16), r.getrandbits(32)]))
    return Call(address, args, regions, conv='s', ret='a')


OPS = [k for k in os.environ.get('OPS', ','.join(CALLS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
