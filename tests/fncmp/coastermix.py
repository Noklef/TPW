# The coaster's mix setters (src/game/coaster/cars: 0x43c3d0, 0x43b2b0) against the originals: values about 0, 100 and the
# word's range (exact and rounded mixes of the type's +0x4c and +0x50), recorded through the global recorder 0x785a08 (recording or
# not, its first stream a record stream with room or none) or quiet; the type's flag 8 set or not (the coaster's contacts cleared:
# nodes 0x100 apart in its +0x54 list, their held contacts followed); for 0x43b2b0 the coaster table 0x790bd0's entry null or not,
# its +0x30 equal to the value or not, its flag 0x8000 with the recorder playing back (+0x24) or not.
# MixPin (0x447e30): MixAt, then the entry's flag 0x8000.
# usage: coastermix.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Mix,MixAt,MixPin)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import RAM, STUBS, Bench, Call, put  # noqa: E402

B = Bench([(0x403d60, 0x403de0), (0x4346c0, 0x434700), (0x43ad50, 0x43af80), (0x43afa0, 0x43afe0), (0x43b2b0, 0x43b300),
           (0x43c3d0, 0x43c460), (0x447e10, 0x447e57), (0x5da3c0, 0x5da3c1)])
RECORDER, TABLE = 0x785a08, 0x790bd0
COAST, TYPE, NODES, VT, BUF = RAM, RAM + 0x400, RAM + 0x1000, RAM + 0x800, RAM + 0x900
OPS = [k for k in os.environ.get('OPS', 'Mix,MixAt,MixPin').split(',') if k]


def recorder(r):
    """The global recorder: its flags, its first stream a record stream (vtable VT, cursor BUF, bytes left), +0x24 playing back."""
    o = bytearray(r.bytes(0x30))
    struct.pack_into('<2I', o, 0, r.choice([0, 1, 1, 1, r.getrandbits(32)]), r.choice([0, 0, 1, 2, r.getrandbits(32)]))
    struct.pack_into('<3I', o, 0xc, VT, BUF, r.choice([0, 2, 4, 0x20, 0x20, 0x20]))
    struct.pack_into('<I', o, 0x24, r.choice([0, 0, 1, r.getrandbits(32)]))
    vt = bytearray(r.bytes(0x10))
    struct.pack_into('<I', vt, 0xc, STUBS + 0x10)
    return {RECORDER: o, VT: vt, BUF: r.bytes(0x20)}


def contacts(r, owner):
    """Nodes 0x100 apart in the owner's +0x54 list (+0x64 next, +8 flags, +0xb0 count, +0xb4 contacts: held ones followed)."""
    n = r.randrange(1, 5)
    nodes = bytearray(r.bytes(0x500))
    ptrs = [NODES + k * 0x100 for k in range(n)]
    listed = [k for k in range(n) if r.random() < 0.8] if r.random() < 0.8 else []
    for i, k in enumerate(listed):
        put(nodes, k * 0x100 + 0x64, 'I', ptrs[listed[i + 1]] if i + 1 < len(listed) else 0)
    put(owner, 0x54, 'I', ptrs[listed[0]] if listed else 0)
    for k in range(n):
        b = k * 0x100
        c = r.randrange(0, 5)
        put(nodes, b + 0xb0, 'I', c)
        for j in range(8):
            put(nodes, b + 0xb4 + 4 * j, 'I', r.choice(ptrs) if j < c else r.choice([0, r.getrandbits(32)]))
    return nodes


def value(r):
    return r.choice([0, 1, 50, 99, 100, 101, 200, r.randrange(0x100), 0xffff, 0x10000, 0x12345, 0x7fffffff, 0x80000000,
                     0xffffffff, r.getrandbits(32)])


def case(r):
    op = r.choice(OPS)
    regions = recorder(r)
    coast = bytearray(r.bytes(0x200))
    put(coast, 0, 'I', r.getrandbits(32) & ~0x8000 | r.choice([0, 0x8000]))
    put(coast, 4, 'I', TYPE)
    v = value(r)
    put(coast, 0x30, 'I', r.choice([v, value(r), value(r)]))
    typ = bytearray(r.bytes(0x60))
    put(typ, 0, 'I', r.getrandbits(32) & ~8 | r.choice([0, 8]))
    for off in (0x4c, 0x50):
        put(typ, off, 'f', r.choice([0.0, 1.0, -1.0, 0.5, 3.0, 100.0, r.uniform(-10, 10), r.uniform(-1e6, 1e6)]))
    regions.update({COAST: coast, TYPE: typ, NODES: contacts(r, coast)})
    if op == 'Mix':
        return Call(0x43c3d0, [COAST, v, r.choice([0, 0, 1, r.getrandbits(32)])], regions, conv='c')
    count = 8
    table = bytearray(4 * count)
    for i in range(count):
        put(table, 4 * i, 'I', r.choice([COAST, COAST, 0]))
    regions[TABLE] = table
    return Call(0x43b2b0 if op == 'MixAt' else 0x447e30, [r.randrange(count), v], regions, conv='c')


if __name__ == '__main__':
    B.main(case)
