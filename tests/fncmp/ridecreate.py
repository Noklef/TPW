# A ride's instance made (src/game/world/rides: Ride_Create 0x463060) against the original. Its calls are recorders (the driver's
# RidesEnv): Instance_Clone 0x4557c0 (cdecl 1 of 5: answering 1 mostly, writing the instance through its out argument), Animator_Reset
# 0x473e30 (2 of 4), Animator_HasMotion 0x472d10 (3 of 3, its &instance logged as 0; 0 or 1), Animator_Play 0x4732a0 (4 of 6),
# Terrain_FootprintHeight 0x466d70 (5 of 7, a float in st0), Animator_Place 0x467030 (6 of 10), Model_FitToGrid 0x469940 (7 of 4),
# ObjModel_BlendTerrainTiles 0x57ace0 (thiscall 8 of 4), Model_TransformBounds 0x46ed80 (9 of 2), StripAnim_FitGround 0x454190 (10 of 1),
# Holder_AddToGrid 0x45bcb0 (11 of 3), Holder_Wake 0x45bd00 (12 of 1), PartStates_Update 0x44ab30 (13 of 1), CoasterLink_Create 0x4368f0
# (14 of 6), Animator_SetPlaying 0x472f40 (15 of 1), Highlight_Flash 0x469580 (16 of 1) and Model_UpdateNormals 0x473c30 (17 of 1);
# Terrain_RotateFootprint, Table7ae6a8_Bit18, WorldVerts_Fetch, Vec3_Cross / Normalize / TurnY run as they are. The prototypes 0x7ae6a8
# (type 0 or an empty entry now and then; +0xd0 another or 0), the ride table 0x7a4610 with its count, holes and first hole (or a ride
# given), the flags (the callers' or random; 8 and 0x200 only with one of 0x26, as every caller: without, the original reads stack
# garbage), the description (x, y in the map, an angle of 0 / 90 / 180 / 270 or another, a size or 0), the map (0x7a0854 -> +0x6c: 16
# cells a row, its heights and cells), 0x7a1a8c's 0x4000, the object (+8: byte flag 0x40, the angle, the cell words, +0x1c, +0x28, a
# frame with triangles, flat now and then so that some normals point up), the root's position (cells 3 to 12 or so, a NaN now and then)
# and the model's entries.
# usage: ridecreate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put, set_float_return, set_return, strings  # noqa: E402

HOOKS = {0x4557c0: _recorder(1, 5, cdecl=True), 0x473e30: _recorder(2, 4, cdecl=True), 0x472d10: _recorder(3, 3, cdecl=True),
         0x4732a0: _recorder(4, 6, cdecl=True), 0x466d70: _recorder(5, 7, cdecl=True), 0x467030: _recorder(6, 10, cdecl=True),
         0x469940: _recorder(7, 4, cdecl=True), 0x57ace0: _recorder(8, 4), 0x46ed80: _recorder(9, 2, cdecl=True),
         0x454190: _recorder(10, 1, cdecl=True), 0x45bcb0: _recorder(11, 3, cdecl=True), 0x45bd00: _recorder(12, 1, cdecl=True),
         0x44ab30: _recorder(13, 1, cdecl=True), 0x4368f0: _recorder(14, 6, cdecl=True), 0x472f40: _recorder(15, 1, cdecl=True),
         0x469580: _recorder(16, 1, cdecl=True), 0x473c30: _recorder(17, 1, cdecl=True)}
RANGES = [(0x463060, 0x46395c), (0x436820, 0x436850), (0x43a9a0, 0x43aa20), (0x4508b0, 0x450910), (0x463a10, 0x463ab0),
          (0x466f90, 0x467030)]
B = Bench(RANGES, hooks=HOOKS)
(PROTO, PROTO2, INST, OBJ, MODEL, ROOT, ENTRIES, WORLD, MAP, HEIGHTS, CELLS, FOOT, WORDS, BLEND, FRAME, VERTS, GROUPS, TRIS, IDX, OUT,
 DESC) = (RAM + k * 0x1000 for k in range(21))
CALLERS = [0x3fa, 0x101, 0xc01, 0x361, 0x32f, 0x33a]
ANGLES = [0, 90, 180, 270]


def case(r):
    log = log_region()
    set_return(log, 1, 1 if r.random() < 0.95 else r.choice([0, 2]))
    fill(log, 1, 3, struct.pack('<I', INST))
    strings(log, 1, 0, out=1 << 3)
    strings(log, 3, 0, out=1)
    set_return(log, 3, r.choice([0, 1]))
    height = r.f(100) if r.random() < 0.9 else r.choice([0x7fc00000, 0xffc00001, 0xff800000])   # (st0 holds no signalling NaN)
    if height & 0x7f800000 == 0x7f800000 and height & 0x7fffff:
        height |= 0x400000
    set_float_return(log, 5, struct.unpack('<f', struct.pack('<I', height))[0])
    set_return(log, 8, r.getrandbits(32))

    flags = r.choice(CALLERS) if r.random() < 0.4 else r.getrandbits(12)
    if flags & 0x208 and not flags & 0x26:
        flags |= r.choice([2, 4, 0x20])
    types = [0] + [PROTO if r.random() < 0.9 else 0 for _ in range(7)]
    t = r.randrange(1, 8) if r.random() < 0.97 else 0
    proto = bytearray(r.bytes(0x100))
    put(proto, 0xd0, 'I', PROTO2 if r.random() < 0.5 else 0)
    put(proto, 0xcc, 'I', FOOT)

    inst = bytearray(r.bytes(0x100))
    put(inst, 0, 'I', PROTO)
    put(inst, 8, 'I', OBJ)
    put(inst, 0xf0, '2I', r.randrange(1, 5), r.randrange(1, 5))
    obj = bytearray(r.bytes(0x40))
    obj[0] = r.getrandbits(8)
    angle = r.choice(ANGLES) if r.random() < 0.9 else r.choice([45, 360, -90 & 0xffffffff, r.getrandbits(32)])
    put(obj, 4, 'I', MODEL)
    put(obj, 0x10, 'I', angle)
    put(obj, 0x18, 'I', WORDS)
    put(obj, 0x1c, 'I', BLEND if r.random() < 0.8 else 0)
    put(obj, 0x28, 'I', r.choice([0, r.getrandbits(32)]))
    put(obj, 0x2c, 'I', FRAME if r.random() < 0.7 else 0)
    model = bytearray(r.bytes(0xc0))
    n_entries = r.randrange(0, 4)
    put(model, 0x44, 'H', n_entries)
    put(model, 0x70, 'I', ENTRIES)
    put(model, 0x78, 'I', ROOT)
    root = bytearray(r.bytes(0x60))
    size = r.choice([10.0, 1.0, 2.5])
    for o in (0x40, 0x48):
        put(root, o, 'f', size * r.uniform(3, 12.9))
    if r.random() < 0.03:
        put(root, r.choice([0x40, 0x48]), 'I', 0x7fc00000)
    world = bytearray(r.bytes(0x70))
    put(world, 0x6c, 'I', MAP)
    mapb = bytearray(r.bytes(0x30))
    put(mapb, 0x10, '2f', size, size)
    put(mapb, 0x18, 'I', 16)
    put(mapb, 0x28, 'I', HEIGHTS)
    put(mapb, 0x2c, 'I', CELLS)
    heights = bytearray(r.bytes(17 * 17 * 4))
    r.floats(heights, range(0, 17 * 17 * 4, 4), 50)
    for _ in range(3):
        put(heights, 4 * r.randrange(17 * 17), 'I', r.choice([0x7f800001, 0x7fc00000, 0xff800000]))
    foot = bytearray(r.bytes(0x10))
    put(foot, 8, '2H', r.randrange(0, 4), r.randrange(0, 4))
    words = bytearray(r.bytes(0x24))
    for k in range(9):
        w = r.getrandbits(16)
        if r.random() < 0.7:
            w = w & ~0x38 | r.choice([0, 8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38])
        put(words, 4 * k, 'H', w)

    frame = bytearray(r.bytes(0xa0))
    put(frame, 0x5a, 'H', r.randrange(0, 4))
    put(frame, 0x60, 'I', VERTS)
    put(frame, 0x6c, 'I', GROUPS)
    put(frame, 0x70, 'I', TRIS)
    put(frame, 0x94, 'I', IDX)
    groups = bytearray(r.bytes(0x30))
    for g in range(3):
        put(groups, 0x10 * g + 4, '2H', r.randrange(0, 7), r.randrange(0, 4))
    tris = bytearray(r.bytes(9 * 8))
    for k in range(9):
        put(tris, 8 * k + 2, '3H', r.randrange(8), r.randrange(8), r.randrange(8))
    idx = struct.pack('<8H', *[r.randrange(8) for _ in range(8)])
    verts = bytearray(0x60)
    r.floats(verts, range(0, 0x60, 4), 20)
    if r.random() < 0.4:
        y = r.f(20)
        for g in range(2):
            for lane in range(4):
                put(verts, 0x30 * g + 0x10 + 4 * lane, 'I', y)

    count = r.randrange(0, 20)
    holes = r.randrange(0, 5) if r.random() < 0.7 else 0
    table = [0] + [INST + 0x10 * k + 0x10000 for k in range(1, 64)]
    total = count + holes
    for k in r.sample(range(1, total + 1), holes) if holes else []:
        table[k] = 0
    for k in range(total + 1, 64):
        table[k] = 0 if r.random() < 0.8 else table[k]
    zeros = [k - 1 for k in range(1, total + 1) if not table[k]]
    hole = count if not holes else (min(zeros) if r.random() < 0.8 else r.randrange(0, total + 1))
    ride = 0xffffffff if r.random() < 0.7 else r.randrange(1, 40)

    desc = struct.pack('<4I', r.randrange(0, 15), r.randrange(0, 15), r.choice(ANGLES) if r.random() < 0.9 else r.getrandbits(32),
                       r.choice([0, r.getrandbits(32) | 1]))
    regions = {LOG: log, PROTO: proto, PROTO2: r.bytes(0x10), INST: inst, OBJ: obj, MODEL: model, ROOT: root,
               ENTRIES: r.bytes(0xa0 * n_entries) if n_entries else r.bytes(4), WORLD: world, MAP: mapb, HEIGHTS: heights,
               CELLS: r.bytes(0x400), FOOT: foot, WORDS: words, BLEND: r.bytes(0x150), FRAME: frame, VERTS: verts, GROUPS: groups,
               TRIS: tris, IDX: idx, OUT: r.bytes(0x70), DESC: desc,
               0x7ae6a8: struct.pack('<8I', *types), 0x7aecd4: struct.pack('<I', OUT), 0x7a4610: struct.pack('<64I', *table),
               0x7aedf4: struct.pack('<I', hole), 0x7a4084: struct.pack('<I', count), 0x7a3ebc: struct.pack('<I', holes),
               0x7a1a8c: struct.pack('<I', r.getrandbits(32)), 0x7a0854: struct.pack('<I', WORLD)}
    args = [t, flags, DESC, r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), r.getrandbits(32), ride,
            r.getrandbits(32)]
    return Call(0x463060, args, regions, ret='a')


if __name__ == '__main__':
    B.main(case)
