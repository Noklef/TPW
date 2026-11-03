# A ride base's cell table (src/game/model/basecells: RideBase_FillCells 0x46aa10) against the original. MapGrid_MarkEdges, GlobalMem_Alloc, WorldVerts_TransformFrame and RideBase_TidyAllUVs are cdecl
# recorders (the driver's BaseCellsEnv): the marked grid and the block from set_return. Debug_Printf and __chkstk run as they are. The fixture: right triangles along the axes, each inside a cell of
# the width by height the call asks for (out of it the original reads and writes its frame's uninitialised rows), so that an edge lies along x and one along z; some wider ones and degenerate ones.
# usage: basecells.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

HOOKS = {0x52c420: _recorder(1, 1, cdecl=True), 0x45b9d0: _recorder(2, 2, cdecl=True), 0x4628b0: _recorder(3, 1, cdecl=True), 0x4679d0: _recorder(4, 5, cdecl=True)}
B = Bench([(0x46aa10, 0x46b3a8), (0x67a830, 0x67a860), (0x67b110, 0x67b13f), (0x5da3c0, 0x5da3d0)], hooks=HOOKS)
OBJECT, PART, HOLDER, MODEL, AREA, MAT, WORLD = RAM, RAM + 0x40, RAM + 0x80, RAM + 0x100, RAM + 0x200, RAM + 0x240, RAM + 0x280
GROUPS, TRIS, MAP, POINTS, UVS, BLOCK, GRID, CELLMAP = RAM + 0x300, RAM + 0x400, RAM + 0x600, RAM + 0x800, RAM + 0x1000, RAM + 0x1800, RAM + 0x2000, RAM + 0x3000
NAN = 0x7fc12345


def triangle(r, width, height, sx, sz):
    """Three corners in cell (cx, cz), in some order: mostly with one edge along x and one along z."""
    cx, cz = r.randrange(width), r.randrange(height)
    x, z = (cx + r.uniform(0.1, 0.4)) * sx, (cz + r.uniform(0.1, 0.4)) * sz
    w, h = r.uniform(0.1, 0.5) * sx * r.choice([1, 1, 1, 0.5]), r.uniform(0.1, 0.5) * sz * r.choice([1, 1, 1, 0.5])
    y = r.uniform(-5, 5)
    kind = r.randrange(15)
    if kind < 9:
        # the right angle at any of the four corners
        cornerx, cornerz = r.choice([(x, x + w), (x + w, x)]), r.choice([(z, z + h), (z + h, z)])
        corners = [(cornerx[0], y, cornerz[0]), (cornerx[1], y, cornerz[0]), (cornerx[0], y, cornerz[1])]
        if kind & 2:
            corners = [(a + r.uniform(-0.01, 0.01), b + r.uniform(-0.3, 0.3), c + r.uniform(-0.01, 0.01)) for a, b, c in corners]
    elif kind == 9:
        corners = [(x, y, z), (x, y, z), (x + w, y, z + h)]
    elif kind == 10:   # along x: no edge along z, so the across best alone passes 0.9
        corners = [(x, y, z), (x + w, y, z + w / 64), (x + w / 2, y, z + w / 32)]
    elif kind in (11, 12):   # two edges tied (exact: quarters), long along x or along z
        cx, cz = (cx + 0.25) * sx, (cz + 0.25) * sz
        a, b = sx / 4, sz / 16
        corners = [(cx, y, cz + b), (cx + a, y, cz), (cx + a, y, cz + 2 * b)]
        if kind == 12:
            a, b = sx / 16, sz / 4
            corners = [(cx + a, y, cz), (cx, y, cz + b), (cx + 2 * a, y, cz + b)]
    else:
        corners = [(x, y, z), (x + w, y + w, z + h / 2), (x + w / 2, y - h, z + h)]
    start = r.randrange(3)
    corners = corners[start:] + corners[:start]
    return corners[::-1] if r.random() < 0.5 else corners


def case(r):
    log = log_region()
    width, height = r.choice([(1, 1), (r.randrange(1, 7), r.randrange(1, 7)), (r.randrange(1, 7), r.randrange(1, 7))])   # (one cell: its triangles compete)
    sx, sz = r.choice([10.0, 16.0, 64.0]), r.choice([10.0, 16.0, 64.0])
    nans = r.choice([0.03, 0.03, 0.25])
    groups = r.randrange(0, 4)
    points, tris, group_bytes, uvs = bytearray(0x600), bytearray(0x180), bytearray(0x40), bytearray(r.bytes(0x800))
    map_words = bytearray(0x200)
    used = 0
    for g in range(groups):
        n = r.randrange(0, 5)
        put(group_bytes, 0x10 * g, 'IHH', MAT + 8 * r.randrange(4), used, n)
        for t in range(n):
            corners = triangle(r, width, height, sx, sz)
            base = 3 * used
            put(tris, 8 * used, '4H', r.getrandbits(16), base, base + 1, base + 2)
            for i, c in enumerate(corners):
                put(map_words, 2 * (base + i), 'H', 47 - base - i)
                put(points, 12 * (47 - base - i), '3f', *c)
                block, slot = (base + i) >> 2, (base + i) & 3
                put(uvs, 0x20 * block + 4 * slot, 'I', r.choice([struct.unpack('<I', struct.pack('<f', r.uniform(-4, 4)))[0], struct.unpack('<I', struct.pack('<f', r.randrange(0, 3)))[0]]) if r.random() >= nans else NAN)
                put(uvs, 0x20 * block + 0x10 + 4 * slot, 'I', r.choice([struct.unpack('<I', struct.pack('<f', r.uniform(-4, 4)))[0], struct.unpack('<I', struct.pack('<f', r.randrange(0, 3)))[0]]) if r.random() >= nans else NAN)
            used += 1
    model = bytearray(r.bytes(0x100))
    put(model, 0x5a, 'H', groups)
    put(model, 0x68, '3I', UVS, GROUPS, TRIS)
    put(model, 0x94, 'I', MAP)
    own = r.random() < 0.5
    part = bytearray(r.bytes(0x40))
    put(part, 4, 'I', HOLDER)
    put(part, 0x2c, 'I', MODEL if own else 0)
    holder = bytearray(r.bytes(0x80))
    put(holder, 0x50, 'I', MAT - r.choice([0, 8, 16]))
    put(holder, 0x78, 'I', MODEL)
    grid = bytes(r.choice([0, 0, 1, 2]) for _ in range(0x140))
    cellmap = bytearray(0x10 + 8 * 400)
    for i in range(400):
        put(cellmap, 8 + 8 * i, 'I', r.choice([0, r.getrandbits(32) | 1, r.getrandbits(32) | 1]))
    set_return(log, 1, GRID if r.random() < 0.8 else 0)
    set_return(log, 2, BLOCK)
    regions = {LOG: log, OBJECT: struct.pack('<16I', *[PART if i == 2 else r.getrandbits(32) for i in range(16)]), PART: part, HOLDER: holder, MODEL: model, AREA: bytes(0x10) + struct.pack('<2f', sx, sz) + bytes(8),
               MAT: r.bytes(0x40), GROUPS: group_bytes, TRIS: tris, MAP: map_words, POINTS: points, UVS: uvs, BLOCK: bytes(width * height * 4), GRID: grid, CELLMAP: cellmap,
               WORLD: bytes(0x6c) + struct.pack('<I', AREA), 0x7aecd4: struct.pack('<I', POINTS), 0x7a0854: struct.pack('<I', WORLD)}
    return Call(0x46aa10, [OBJECT, CELLMAP, width, height, r.choice([0, 0, 0, 1])], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
