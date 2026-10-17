# The ride state predicates (src/game/world/ridestate: 0x42a440) against the originals, with the callees run: a ride object or none, its track
# ride handle (0x877b58 module loaded or not, the handle naming its ride or not, the link flag 0x400000 and value), its ride record (0x8791a0 /
# 0x8791b0) naming a flying ride (0x8791f8) whose first slot holds a ride or not, the ride index's bit 18 and coaster link (0x790fe0) value, and
# its holder's model parts (key 1 among the 0x1000 parts or not).
# usage: ridestate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x42a440, 0x42a4f0), (0x436840, 0x436870), (0x436880, 0x4368d0), (0x44b220, 0x44b280), (0x549d30, 0x549dc0), (0x55a070, 0x55a0c0),
           (0x55a360, 0x55a3a0), (0x55a570, 0x55a5b0), (0x5da3c0, 0x5da3c1)])
OBJ, TRACK, TLINK, NODES, FLY, HOLDERS, MODELS, PARTS, LINKS = (RAM + o for o in (0, 0x100, 0x500, 0x600, 0x1000, 0x2000, 0x2400, 0x2800, 0x3000))


def maybe(r, value, p=0.5):
    return value if r.random() < p else 0


def case(r):
    regions = {}
    # the track rides: four rides of 0xd0 bytes, their serial at +0 and link at +0x58
    loaded = r.random() < 0.8
    regions[0x877b58] = struct.pack('<3I', 0x4a454647 if loaded else r.getrandbits(32), 0, TRACK)
    track = bytearray(r.bytes(0xd0 * 4))
    for i in range(4):
        put(track, 0xd0 * i, 'I', r.choice([0, 1, r.getrandbits(24)]))
        put(track, 0xd0 * i + 0x58, 'I', r.choice([TLINK, TLINK, 0]))
    tlink = bytearray(r.bytes(0x10))
    put(tlink, 0, 'I', r.choice([0x400000, 0x400001, 0, r.getrandbits(32)]))
    put(tlink, 8, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions.update({TRACK: track, TLINK: tlink})
    t = r.randrange(4)
    serial = struct.unpack_from('<I', track, 0xd0 * t)[0]
    handle = r.choice([t | serial << 8 & 0xffffffff] * 3 + [t | (serial ^ 1) << 8 & 0xffffffff, 0, 0])
    # the ride records: ids 1-3, +0x9c the flying ride index
    nodes = bytearray(r.bytes(0x100 * 3))
    count = r.choice([0, 1, 2, 3])
    for i in range(count):
        put(nodes, 0x100 * i, 'I', NODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, 0x100 * i + 8, 'I', i + 1)
        put(nodes, 0x100 * i + 0x9c, 'i', r.choice([0, 1, 2, 3, 99, 100, -1]))
    regions.update({NODES: nodes, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1])), 0x8791b0: struct.pack('<I', NODES if count else 0)})
    # the flying rides: records 1 and 2 (record 3 absent), the first slot's ride at +0x11c
    table = bytearray(400)
    put(table, 4, 'I', FLY)
    put(table, 8, 'I', FLY + 0x200)
    fly = bytearray(r.bytes(0x400))
    put(fly, 0x11c, 'I', maybe(r, r.getrandbits(32) | 1))
    put(fly, 0x200 + 0x11c, 'I', maybe(r, r.getrandbits(32) | 1))
    regions.update({0x8791f8: table, FLY: fly})
    # the ride holders 0-3: +4 flags (bit 18), +8 -> +4 the model: +0x48 the part count, +0x7c the parts (+0 flags, +4 key)
    holders = bytearray(r.bytes(0x40 * 4))
    models = bytearray(r.bytes(0x100 * 4))
    parts = bytearray(r.bytes(0x14 * 4 * 4))
    for i in range(4):
        put(holders, 0x40 * i + 4, 'I', (r.getrandbits(32) & ~0x40000) | maybe(r, 0x40000))
        put(holders, 0x40 * i + 8, 'I', HOLDERS + 0x40 * i + 0x20)
        put(holders, 0x40 * i + 0x24, 'I', MODELS + 0x100 * i)
        put(models, 0x100 * i + 0x48, 'H', r.choice([0, 1, 2, 4]))
        put(models, 0x100 * i + 0x7c, 'I', PARTS + 0x50 * i)
        for k in range(4):
            o = 0x50 * i + 0x14 * k
            put(parts, o, 'I', r.choice([0x1000, 0x1001, 0, 1, r.getrandbits(32)]))
            put(parts, o + 4, 'I', r.choice([1, 1, 0, 2]))
    regions.update({HOLDERS: holders, MODELS: models, PARTS: parts, 0x7a4610: b''.join(struct.pack('<I', HOLDERS + 0x40 * k) for k in range(4))})
    # the coaster links: ids 0-3 or others, +0 flags (0x800), +0x124 -> +0x34 -> +0xc the value
    links = bytearray(r.bytes(0x200 * 2))
    nl = r.choice([0, 1, 2])
    for i in range(nl):
        o = 0x200 * i
        put(links, o, 'I', (r.getrandbits(32) & ~0x800) | maybe(r, 0x800, 0.7))
        put(links, o + 0x10, 'I', r.choice([0, 1, 2, 3, 7]))
        put(links, o + 0x18, 'I', LINKS + 0x200 * (i + 1) if i + 1 < nl else 0)
        put(links, o + 0x124, 'I', LINKS + o + 0x140)
        put(links, o + 0x140 + 0x34, 'I', LINKS + o + 0x180)
        put(links, o + 0x180 + 0xc, 'I', r.choice([0, 1, r.getrandbits(32)]))
    regions.update({LINKS: links, 0x790fe0: struct.pack('<I', LINKS if nl else 0)})
    obj = bytearray(r.bytes(0x40))
    put(obj, 0x24, 'i', r.choice([0, 1, 2, 3, 9]))
    put(obj, 0x28, 'I', handle)
    regions[OBJ] = obj
    return Call(0x42a440, [OBJ if r.random() < 0.85 else 0, r.randrange(4)], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
