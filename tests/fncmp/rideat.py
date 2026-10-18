# The ride at a map cell (src/game/terrain/land: Map_RideIndexAt 0x452a60, Terrain_RideAt 0x450ac0) against the originals, over the
# originals of their lookups (MapCell_Dword4At 0x53bf30, Map_ObjectOrEntranceAt 0x527e80, Map_ObjectAtCell 0x527d60, Cell_FirstObject
# 0x4d99e0, Map_InBounds 0x4d8300): a world (0x7cf83c) with both cell tables over the first four rows (the 0x28-byte cells' dword +4 often
# 0, their types often 0xc / 0x11 with a parent; the 0x44-byte cells' types often 4 / 9 / 0xa, parents and object chains), the entrance's
# row and columns about the cell, and a thing table whose things are rides (kind 3) or not, one slot empty; the terrain grid (0x7a0854
# +0x6c) small or wide, the cell beyond it or not.
# Surface (Terrain_SurfaceHeight 0x452ae0, over Terrain_HeightAt 0x4527f0): that world with a slot (null, 0, a ride or a bad index), the
# rides' records present or not, with height maps or not, their origins about the cell's position (now and then NaN or off by a little
# more), turned 0, 90, 180, 270 or otherwise; the ground a 16 x 8 grid of 10-unit cells.
# usage: rideat.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, put  # noqa: E402

B = Bench([(0x452a60, 0x452ad7), (0x450ac0, 0x450b00), (0x53bf30, 0x53bfa0), (0x527e80, 0x527ed8), (0x527d60, 0x527e76),
           (0x4d99e0, 0x4d9a5b), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1), (0x452ae0, 0x452c97),
           (0x4527f0, 0x452a70)])
WORLD = 0x32000000
IDS = 0x220   # cell ids 0 - 0x21f: rows 0 - 3 and a little past
THINGS, MAP, GRID = RAM, RAM + 0x4000, RAM + 0x4100
NSLOT = 16    # handles 1 - 15; slot 15 empty


def world(r, wide=True):   # wide: ride indices of any size (else under 0x10, the ride table's entries At maps)
    regions = {}
    c28 = bytearray(r.bytes(0x28 * IDS))
    for k in range(IDS):
        put(c28, 0x28 * k + 4, 'I', r.choice([0, 0, 0, 1, 5, 15, r.getrandbits(32) if wide else r.randrange(0x10)]))
        put(c28, 0x28 * k + 8, 'I', r.choice([0xc, 0x11, 0, 0, 3, 4, r.getrandbits(32)]))
        put(c28, 0x28 * k + 0x10, 'H', r.choice([0, r.randrange(IDS), r.randrange(IDS)]))
    c44 = bytearray(r.bytes(0x44 * IDS))
    for k in range(IDS):
        put(c44, 0x44 * k + 8, 'I', r.choice([4, 9, 0xa, 4, 9, 0xa, 0, 3, 0xb, r.getrandbits(32)]))
        put(c44, 0x44 * k + 0x10, 'H', r.choice([k, k, k, r.randrange(1, 0x200)]))
        put(c44, 0x44 * k + 0x24, 'H', r.choice([0, r.randrange(1, NSLOT - 1), r.randrange(1, NSLOT - 1)]))
    slots = bytearray(r.bytes(20 * NSLOT))
    things = bytearray(r.bytes(0x40 * NSLOT))
    for h in range(1, NSLOT):
        put(slots, 20 * h, 'I', THINGS + 0x40 * h if h < NSLOT - 1 else 0)
        t = 0x40 * h
        things[t + 2] = r.choice([3, 3, 3, 1, 2, 0xff])
        put(things, t + 0xa, 'H', r.choice([0, r.randrange(h)]))   # (chains run down to 0: no cycles)
        put(things, t + 0x20, 'I', r.choice([0, 1, 7, 15, r.getrandbits(32) if wide else r.randrange(0x10)]))
        put(things, t + 0x34, 'H', r.randrange(1, 0x200))
    regions.update({WORLD + 0x294: c44, WORLD + 0x1102b0: c28, WORLD + 0x1da730: struct.pack('<HH', 0, r.choice([0, 1, 5, NSLOT - 1, r.randrange(NSLOT)])),
                    0x7cf83c: struct.pack('<I', WORLD), 0x7cfb90: slots, THINGS: things})
    return regions


def entrance(r, regions, x, y):
    near = lambda v: v + r.randrange(-2, 3)  # noqa: E731
    first, last = sorted([near(x), near(x)])
    row = y if r.random() < 0.4 else r.randrange(4)
    regions[0x7855bc] = struct.pack('<3i', first, row, last)


def cell(r):
    return r.choice([0, 1, 5, 0x7e, 0x7f, 0x80, r.randrange(0x80), r.randrange(0x80)]), r.choice([0, 1, 2, 3, r.randrange(3)])


def ride_index(r):
    x, y = cell(r)
    if r.random() < 0.05:
        x, y = -1, r.choice([1, 2])
    regions = world(r)
    entrance(r, regions, x, y)
    return Call(0x452a60, [x & 0xffffffff, y], regions, ret='a')


def ride_at(r):
    width = r.choice([1, 2, 0x10, 0x80, 0x80, r.randrange(1, 0x81)])
    height = r.choice([1, 2, 3, 4, 4])
    x = r.choice([0, width - 1, width, width + 1, r.randrange(0x80), 0xffffffff, r.getrandbits(32)])
    y = r.choice([0, height - 1, height, r.randrange(4), 0xffffffff, r.getrandbits(32)])
    regions = world(r, False)
    entrance(r, regions, min(x, width - 1), min(y, height - 1, 3))
    regions[MAP] = put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID)
    regions[GRID] = put(bytearray(r.bytes(0x20)), 0x18, 'II', width, height)
    regions[0x7a0854] = struct.pack('<I', MAP)
    regions[0x7a4610] = r.bytes(4 * 0x10)
    return Call(0x450ac0, [x, y], regions, ret='a')


def surface(r):
    import terrainheight  # noqa: E402  (its heights; its bench is its own)
    x, y = cell(r)
    regions = world(r, False)
    entrance(r, regions, x, y)
    SLOT, RIDES, HMAPS, LEVELS, GROUND = RAM + 0x5000, RAM + 0x5100, RAM + 0x7000, RAM + 0x8000, RAM + 0x10000
    table = bytearray(4 * 0x10)
    for i in range(1, 0x10):
        if r.random() < 0.15:
            continue
        rec = RIDES + 0x100 * i
        put(table, 4 * i, 'I', rec)
        place, obj = rec + 0xd0, rec + 0x80   # (the record's +8 -> place (+0x10 the angle), place +4 -> obj, obj +0x78 -> rec: origin +0x40)
        hmap = HMAPS + 0x20 * i if r.random() < 0.85 else 0
        record = put(put(bytearray(r.bytes(0x100)), 8, 'I', place), 0xcc, 'I', hmap)
        put(record, 0xd4, 'I', obj)
        put(record, 0xe0, 'I', r.choice([0, 90, 180, 270, 90, 180, 270, 45, r.getrandbits(32)]))
        put(record, 0x80 + 0x78, 'I', rec)
        regions[rec] = record
        regions.setdefault(HMAPS, bytearray(r.bytes(0x20 * 0x10)))
        if hmap:
            put(regions[HMAPS], 0x20 * i + 8, 'H', r.choice([1, 4, 16, 16, r.randrange(1, 20)]))
            put(regions[HMAPS], 0x20 * i + 0x10, 'I', LEVELS + 0x1000)
    # origins: about the position, so that the offsets stay within the levels
    for i in range(1, 0x10):
        if RIDES + 0x100 * i in regions:
            ox, oz = (v * 10 + r.uniform(-60, 60) if r.random() < 0.95 else r.choice([float('nan'), v * 10 + 150.0]) for v in (x, y))
            put(regions[RIDES + 0x100 * i], 0x40, '3f', ox, r.uniform(-20, 20), oz)
    regions[0x7a4610] = table
    regions[LEVELS] = r.bytes(0x2000)
    slot = r.choice([0, SLOT, SLOT, SLOT])
    regions[SLOT] = struct.pack('<I', r.choice([0, 0, r.randrange(1, 0x10), r.randrange(1, 0x10)]))
    grid = put(put(put(bytearray(r.bytes(0x30)), 0x10, '2f', 10.0, 10.0), 0x18, '2I', 16, 8), 0x28, '2I', GROUND, GROUND + 0x1000)
    heights = bytearray(4 * 17 * 9)
    for i in range(17 * 9):
        put(heights, 4 * i, 'f', terrainheight.height(r))
    regions.update({0x7a0854: struct.pack('<I', MAP), MAP: put(bytearray(r.bytes(0x70)), 0x6c, 'I', GRID), GRID: grid, GROUND: heights,
                    GROUND + 0x1000: r.bytes(0x400)})
    return Call(0x452ae0, [x & 0xffffffff, y, slot], regions, ret='d')


CASES = {'Index': ride_index, 'At': ride_at, 'Surface': surface}


def case(r):
    return CASES[r.deal(OPS)](r)


OPS = [k for k in os.environ.get('OPS', ','.join(CASES)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
