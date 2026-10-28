# A track cell's messages (src/game/world/trackmessage: MapCell_HandleTrackMessage 0x5370e0) against the original. Its calls are
# recorders (the driver's TrackMessageEnv): MapCell_CanBuildTrack 0x539760 (thiscall 1 of 3, its direction logged as a byte; answers 0,
# 1, 2, 5 or 0xd), DynFaces_AddSquare 0x53c8d0 (cdecl 2 of 11, d and h bytes), TrackCell_Remove 0x53b280 (3), MapLine_LinkCell
# 0x539220 (4), MapCell_SetStepDirection 0x539670 (5), MapCell_ApplyPathModel 0x5365d0 (6), MapCell_ApplyModel 0x53afa0 (7),
# MapCell_SetTrackKind 0x538fc0 (8; answers 0, 1 or others), TrackRun_UpdateA / B 0x52c6f0 / 0x52ccb0 (cdecl 9 / 10), Path_CheckDeadEnds
# 0x527ee0 (cdecl 11), MapCell_Clear 0x5367a0 (12) and GameMode_SetOnly 0x52f580 (cdecl 13). The rest runs as it is: the heights
# (Terrain_SurfaceHeight over Terrain_HeightAt, Terrain_RoundedCellHeight), the steps (Map_StepCell, Map_StepCell28, IntPair_Copy), the
# cells' accessors, Cell_FirstObject, Global816d48_Set, Global81ae2c_Get and the empty Debug_Printf.
# The world: rideat.py's Surface world (rides, ground, object chains) over rows 0-5 of both cell tables; the 0x28-byte cells' id words
# their own mostly, their types the height, track and hoarding ones or others, their neighbour bits (+0xc, or +0x22), direction (+0xd)
# and flags (+0xe) of the shapes the tests read; the 0x44-byte cells' types 0, 1, 2, 3, 7, 0x15, 0x1e and others; the step pairs
# 0x7cdba0-0x7cdbd8 steps of -1 to 1; the cell in rows 2-3 within the ground's columns. The messages: walls (0x100 | a code), 0x32 and
# 0x80-0x87, others (0x19 often), with bits past the low byte now and then; a and b -2, 2, 5 and others; the globals read set or not
# (the points 0x7639f0 / 0x7639f8 the hoarding's own cell now and then).
# usage: trackmessage.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import LOG, LOG_BYTES, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402
import rideat  # noqa: E402

HOOKS = {0x539760: _recorder(1, 3), 0x53c8d0: _recorder(2, 11, cdecl=True), 0x53b280: _recorder(3, 3), 0x539220: _recorder(4, 3),
         0x539670: _recorder(5, 2), 0x5365d0: _recorder(6, 0), 0x53afa0: _recorder(7, 0), 0x538fc0: _recorder(8, 1),
         0x52c6f0: _recorder(9, 4, cdecl=True), 0x52ccb0: _recorder(10, 4, cdecl=True), 0x527ee0: _recorder(11, 3, cdecl=True),
         0x5367a0: _recorder(12, 2), 0x52f580: _recorder(13, 2, cdecl=True)}
RANGES = [(0x5370e0, 0x538eec), (0x452ae0, 0x452c97), (0x4527f0, 0x452a70), (0x53bf30, 0x53bfa0), (0x527e80, 0x527ed8),
          (0x527d60, 0x527e76), (0x4d99e0, 0x4d9a5b), (0x4d8300, 0x4d8328), (0x5da3c0, 0x5da3c1), (0x536ca0, 0x5370dd),
          (0x4d96f0, 0x4d97e0), (0x4d99c0, 0x4d99e0), (0x4d0af0, 0x4d0b40), (0x50c320, 0x50c3f0), (0x522700, 0x5228b5),
          (0x53a510, 0x53a540), (0x536300, 0x53644c), (0x527940, 0x527950), (0x52f840, 0x52f850)]
B = Bench(RANGES, hooks=HOOKS)
WORLD = rideat.WORLD
IDS = 0x310   # rows 0-5 and a little past
C28, C44 = WORLD + 0x1102b0, WORLD + 0x294
TYPES28 = [0xb, 0x10, 0x19, 7, 4, 9, 0xa, 0xd, 0x12, 0xc, 0x11, 0, 3, 0x19, 0x19, 0xb]
TYPES44 = [0, 1, 2, 3, 7, 0x1e, 0x15, 4, 9, 0, 1, 3]
NEIGHBOURS = [0x41, 0x14, 5, 0x50, 0x11, 0x44, 0x55, 0x15, 1, 0, 0x45]
SIDES = [1, 4, 0x10, 0x40]
SPECIAL = [0x32, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87]


def high(r):
    return r.choice([0, 0, 0, 0, 0x400, 0x200, 0xff0000, r.getrandbits(32) & ~0x1ff])


def world(r):
    rideat.IDS = IDS
    regions = dict(rideat.surface(r).regions)
    c28, c44 = bytearray(regions[C28]), bytearray(regions[C44])
    for k in range(IDS):
        e = 0x28 * k
        put(c28, e, 'H', k if r.random() < 0.95 else r.randrange(1, IDS))
        c28[e + 2] = r.choice([2, 0, 1, r.getrandbits(8)])
        put(c28, e + 8, 'I', r.choice(TYPES28) if r.random() < 0.95 else r.getrandbits(32))
        c28[e + 0xc] = r.choice(NEIGHBOURS) if r.random() < 0.8 else r.getrandbits(8)
        c28[e + 0x22] = r.choice(NEIGHBOURS) if r.random() < 0.8 else r.getrandbits(8)
        c28[e + 0xd] = r.choice(SIDES) if r.random() < 0.85 else r.getrandbits(8)
        put(c28, e + 0xe, 'H', r.choice([0, 0x40, 0xf, 0x4f, r.getrandbits(16)]))
        c = 0x44 * k
        if r.random() < 0.95:
            put(c44, c, 'H', k)
        put(c44, c + 8, 'I', r.choice(TYPES44) if r.random() < 0.9 else r.getrandbits(32))
        put(c44, c + 0xe, 'H', r.choice([0, 0x40, 0x41, r.getrandbits(16)]))
    regions[C28], regions[C44] = c28, c44
    pairs = bytearray(0x38)
    for k in range(0, 0x38, 4):
        put(pairs, k, 'i', r.choice([-1, 0, 1, 1, -1]))
    regions.update({0x7cdba0: pairs, 0x80239c: struct.pack('<I', WORLD), 0x8023a0: struct.pack('<I', WORLD + 0x2d8)})
    x, y = r.randrange(2, 14), r.choice([2, 3])
    ident = (y << 7) + x + 1
    cell = C28 + 0x28 * ident
    put(c28, 0x28 * ident, 'H', ident)
    flag = lambda: struct.pack('<I', r.choice([0, 0, 1, r.getrandbits(32)]))  # noqa: E731
    points = []
    for _ in range(2):   # (each point the cell's own x and y now and then, else near it or anywhere)
        if r.random() < 0.3:
            points += [x, y]
        else:
            points += [r.choice([x, r.randrange(16), r.getrandbits(32)]), r.choice([y, r.randrange(6), r.getrandbits(32)])]
    regions.update({0x81b4cc: flag(), 0x816d64: struct.pack('<I', r.getrandbits(32)), 0x820ad0: flag() + bytes(20) + flag(),
                    0x8186c4: flag(), 0x818698: struct.pack('<I', r.getrandbits(32)), 0x816d48: bytes(4),
                    0x818c10: struct.pack('<2i', *r.choice([(5, 4), (4, 5), (5, 5), (0, -1), (r.getrandbits(31), r.getrandbits(31))])),
                    0x81ede4: r.bytes(8), 0x81ae2c: struct.pack('<I', r.choice([0, 1, 2, 3, 4, r.getrandbits(32)])),
                    0x7639f0: struct.pack('<4I', *points)})
    return regions, cell, c28, ident


def case(r):
    regions, cell, c28, ident = world(r)
    log = log_region()
    regions[LOG] = log
    struct.pack_into('<I', log, LOG_BYTES + 4 * 1, 1 << 2)
    struct.pack_into('<I', log, LOG_BYTES + 4 * 2, 1 << 3 | 1 << 8)
    set_return(log, 1, r.choice([0, 1, 1, 1, 2, 5, 0xd, 0xd]))
    set_return(log, 8, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, 9, r.choice([0, 1, 0x100]))
    step = [0xfffffffe, 2, 5, 0, 1, 0xffffffff, r.getrandbits(32)]
    a, b = r.choice(step), r.choice(step)
    pick = r.random()
    if pick < 0.35:
        code = r.choice([4, 4, 0xb, 0x10, 0x19, 0xe, 0x13, 0xf, r.getrandbits(8)])
        message = 0x100 | code | r.choice([0, 0, 0, 0x200, 0xff0000])
        if code in (0xb, 0x10) and r.random() < 0.5:
            a = 5
    elif pick < 0.85:
        message = r.choice(SPECIAL + [0x81, 0x81, 0x85]) | high(r)
    else:
        message = r.choice([0x19, 0x19, 0xb, 0xc, 0x15, 0, r.getrandbits(8) & ~0x80]) | high(r) & ~0x100
    code, e = message & 0x1ff, 0x28 * ident
    if code in (0x81, 0x83, 0x85) and r.random() < 0.7:   # (a hoarding or track cell mostly, at an end, a corner or a junction; the
        # model's hoardings at the corners that do not match their direction more often, the end flag's at an end or alone)
        hoarding = r.random() < 0.6 if code == 0x81 else r.random() < 0.35
        put(c28, e + 8, 'I', 0x19 if hoarding else r.choice([0x19, 0xb, 0xc, 0x10, 0x11, 0xd, 0x12, 3, 0]))
        c28[e + 0xd] = r.choice(SIDES)
        shapes = [0x11, 0x44, 1, 4, 0x10, 0x40, 0, 0x14, 0x50] if code == 0x85 else [0x41, 0x14, 5, 0x50, 0x11, 0x44, 0x55, 0x14, 0x50]
        c28[e + 0xc] = c28[e + 0x22] = r.choice(shapes)
    if code == 0x85 and r.random() < 0.4:   # (the 0x44-byte cells about it empty mostly, its own of the types the flag reads)
        c44 = regions[C44]
        for k in (ident + dy * 128 + dx for dy in (-1, 0, 1) for dx in (-1, 0, 1)):
            put(c44, 0x44 * k + 8, 'I', 0 if r.random() < 0.9 else r.choice([1, 2, 7]))
        put(c44, 0x44 * ident + 8, 'I', r.choice([0, 7, 0x1e, 2, 1, 3, 0x15]))
    if code == 0x86 and r.random() < 0.7:
        put(c28, 0x28 * ident + 8, 'I', r.choice([0xb, 0xd, 0x10, 0x12, 0x19]))
        put(c28, 0x28 * ident + 0x10, 'H', r.choice([0, r.randrange(1, IDS)]))
    return Call(0x5370e0, [cell, message, a, b], regions, conv='t', ret='a')


if __name__ == '__main__':
    B.main(case)
