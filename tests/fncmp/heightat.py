# The height of a model's floor under a point (src/game/model/heightat: 0x451390) against the
# original: groups of faces (floor or not) over small triangles around the point, with or
# without an instance's vertex offset and given normals, at each view angle, first or best.
# usage: heightat.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import Bench, Call, RAM, put  # noqa: E402

B = Bench([(0x451390, 0x451634)])
MODEL, GROUPS, FACES, REMAP, NORMALS, GIVEN = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x400, RAM + 0x500, RAM + 0x600
INSTANCE, VIEW, VERTS = RAM + 0x780, RAM + 0x7c0, RAM + 0x800


def case(r):
    ram = bytearray(r.bytes(0xa00))
    groups = r.choice([0, 1, 1, 2, 4])
    put(ram, 0x5a, 'H', groups)
    put(ram, 0x64, 'I', NORMALS)
    put(ram, 0x6c, 'I', GROUPS)
    put(ram, 0x70, 'I', FACES)
    put(ram, 0x94, 'I', REMAP)
    for g in range(groups):
        count = r.choice([0, 1, 3, 8])
        put(ram, 0x100 + 0x10 * g + 4, 'HH', r.randrange(0, 33 - count), count)
    for f in range(32):
        flags = r.randrange(16) | (0x8000 if r.random() < 0.8 else r.getrandbits(15) & 0x7ff0)
        put(ram, 0x200 + 8 * f, 'HHHH', flags, *(r.randrange(16) for _ in range(3)))
    for i in range(16):
        put(ram, 0x400 + 2 * i, 'H', r.randrange(16))
    size = r.choice([1, 4, 100, None])
    r.floats(ram, range(0x500, 0x5c0, 4), r.choice([1, 2, None]))
    r.floats(ram, range(0x600, 0x780, 4), r.choice([1, 2, None]))
    r.floats(ram, range(0x800, 0x800 + 24 * 12, 4), size)
    put(ram, 0x790, 'H', r.randrange(9))
    put(ram, 0x7d0, 'i', r.choice([0, 90, 180, 270, 45, -90, 360]))
    instance = r.choice([0, INSTANCE])
    normals = r.choice([0, 0, GIVEN])
    mode = r.choice(['random', 'grid', 'grid', 'big'])
    if mode != 'random':  # floors on a half grid (flat ones too), or spanning past float range
        for o in range(0x800, 0x800 + 24 * 12, 4):
            put(ram, o, 'f', r.randrange(-8, 9) / 2)
            if mode == 'big' and r.random() < 0.3:
                put(ram, o, 'f', r.choice([3e38, -3e38]))
        if r.random() < 0.5:
            for o in range(0x804, 0x800 + 24 * 12, 12):
                put(ram, o, 'f', r.choice([0.0, -0.0, 0.0, -0.0, 1.5]))
            for o in list(range(0x500, 0x5c0, 12)) + list(range(0x600, 0x780, 12)):
                put(ram, o, 'fff', r.choice([0.0, -0.0]), r.choice([1.0, 2.0]), r.choice([0.0, -0.0]))
    x, z = r.f(size), r.f(size)
    if mode != 'random' and r.random() < 0.6:  # on a vertex, or halfway along an edge
        p, q = (struct.unpack_from('<fff', ram, 0x800 + 12 * r.randrange(24)) for _ in range(2))
        if r.random() < 0.5:
            p = q = [(p[i] + q[i]) / 2 for i in range(3)]
        x, z = (struct.unpack('<I', struct.pack('<f', v))[0] for v in (p[0], p[2]))
    first = r.choice([0, 0, 1, 4])
    regions = {RAM: ram, 0x79f7e0: struct.pack('<I', VERTS), 0x79fb18: struct.pack('<I', VIEW)}
    return Call(0x451390, [MODEL, instance, x, z, normals, first], regions, ret='d')


if __name__ == '__main__':
    B.main(case)
