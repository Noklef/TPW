# A ride base's texture coordinates tidied (src/game/model/baseuvs: 0x467310) against the original. The part at RAM: 0-40 vertices, 0-12
# triangles naming them, 0-4 groups over runs of the triangles (now and then past the last), their materials' flag words any (or none).
# The coordinates mostly near whole numbers (within a few tolerances either side), else any in -4..4, now and then NaN, infinite or huge;
# the tolerance mostly small (0.001-0.2), now and then 0, negative, 0.5 or more, NaN. With a map (16 x 16 bytes, -16..+256 around it,
# mostly set) and positions (each vertex's through the part's u16 map: x in 0..160, z in -160..0, y mostly within 0.1 of 0), or without
# either. warned 0 or 1. The scratch blocks from fncmp's global heap.
# usage: baseuvs.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import HEAP, RAM, Bench, Call, fb, global_heap, heap, put  # noqa: E402

B = Bench([(0x467310, 0x4679cd), (0x45b9d0, 0x45ba15), (0x45ba00, 0x45ba30), (0x5da3c0, 0x5da3c1)])
PART, UVS, TRIS, GROUPS, INDEX, MATS, POS, MAP, MODEL, WARNED = (RAM + 0x400 * k for k in range(10))


def coord(r, tolerance):
    k = r.random()
    if k < 0.6:
        whole = r.choice([0, 0, 1, -1, 2, -2, 3, r.randrange(-50, 50)])
        return fb(whole + r.choice([1, -1]) * r.choice([0, tolerance, 2 * tolerance, 0.5 * tolerance, r.uniform(0, 3 * abs(tolerance) + 1e-6)]))
    if k < 0.92:
        return fb(r.uniform(-4, 4))
    return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000, 0x4f000000, 0xcf000000, r.getrandbits(32)])


def case(r):
    verts = r.choice([0, 1, 3, 4, 5, 8, 12, 20, 40, r.randrange(41)])
    tris = 0 if not verts else r.choice([0, 1, 2, 3, 6, 12, r.randrange(13)])
    groups = r.choice([0, 1, 1, 2, 3, 4])
    tolerance = r.choice([0.01, 0.001, 0.05, 0.1, 0.2, 0.2, 0, -0.05, 0.5, 0.75, 1.5, float('nan'), r.uniform(0, 0.3)])
    uvs = bytearray(r.bytes(0x20 * 10))
    for i in range(verts):
        put(uvs, (i >> 2) * 0x20 + (i & 3) * 4, 'I', coord(r, tolerance))
        put(uvs, (i >> 2) * 0x20 + (i & 3) * 4 + 0x10, 'I', coord(r, tolerance))
    tri = bytearray(r.bytes(8 * 13))
    for k in range(13):   # (the spare ones too: a group running past the last reads them)
        put(tri, 8 * k + 2, '3H', *(r.randrange(max(verts, 1)) for _ in range(3)))
    group = bytearray(r.bytes(0x10 * 4))
    mats = bytearray(r.bytes(0x10))
    for g in range(groups):
        first = r.randrange(tris + 1)
        count = r.choice([tris - first, tris - first, 0, r.randrange(tris - first + 1), min(tris - first + 1, 13 - first)])
        put(group, 0x10 * g, 'I', r.choice([MATS + 2 * g, MATS + 2 * g, MATS + 2 * g, 0]))
        put(group, 0x10 * g + 4, 'HH', first, max(count, 0))
    npos = r.choice([1, 4, 16])
    index = struct.pack('<40H', *(r.randrange(npos) for _ in range(40)))
    pos = bytearray()
    for _ in range(npos):
        y = r.choice([0.0, 0.0, 0.05, -0.05, 0.09, 0.2, -0.3, r.uniform(-0.15, 0.15)])
        pos += struct.pack('<3f', r.uniform(0, 159.9), y, -r.uniform(0, 159.9))
    if r.random() < 0.05:
        put(pos, 4, 'I', 0x7fc00000)
    grid = bytes(r.choice([1, 1, 1, 0, 2]) for _ in range(0x120))
    with_map = r.random() < 0.6
    part = bytearray(r.bytes(0xa0))
    put(part, 0x5a, 'HHH', groups, tris, verts)
    put(part, 0x68, 'III', UVS, GROUPS, TRIS)
    put(part, 0x94, 'I', INDEX)
    regions = {PART: part, UVS: uvs, TRIS: tri, GROUPS: group, MATS: mats, INDEX: index, POS: bytes(pos), MAP: grid, MODEL: r.bytes(0x40),
               WARNED: struct.pack('<I', r.choice([0, 0, 0, 1])), HEAP: heap(0x2000), **global_heap()}
    args = [MODEL, PART, fb(tolerance), MAP + 0x10 if with_map or r.random() < 0.2 else 0, POS if with_map or r.random() < 0.2 else 0, WARNED]
    return Call(0x467310, args, regions, conv='c', ret='v')


if __name__ == '__main__':
    B.main(case)
