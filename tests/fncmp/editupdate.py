# The coaster editor's frame update (src/game/coaster/editupdate: CoasterEdit_Update 0x443340) against the original. Its calls are cdecl
# recorders (the driver's EditUpdateEnv): CoasterLink_FindCarOnRide 0x443ee0 (1; answers a car, the span's end or none),
# TrackSelection_ClearFlags 0x444040 (2), Flags_SetChanged 0x4346c0 (3), TrackSelection_SetFirst 0x444090 (4), CoasterLinks_FindAtFocus
# 0x433f70 (5; answers a piece or none), Coaster_DropCar 0x442f40 (6; writes its two out floats, logged as 0), Car_SetMoveCostChange
# 0x443df0 (7; its x logged as the float it points to, its z as 0), CoasterPiece_CanPlaceAt 0x434740 (8; 0 or 1), Car_Edited 0x443f40 (9)
# and Car_TravelCost 0x447830 (10; a float in st0). Vec3_AngleY, Float_RoundUp and Global81af4c_Add run as they are.
# The world: the sine table (4096 entries, as the game builds it), the editor's block 0x79c600-0x79c700 (flags of a tool or none with
# the adjust bits 0x200 / 0x400, the vector bits 0x800 / 0x1000, the span tool 0x80 or the move tool 0x100, 0x2000 now and then; the
# mouse and its start equal now and then with the last moves 0 so that the moves are unchanged; values about their ranges; vectors;
# the pointers to the values and vectors, the second ones none now and then), a link (+0x40 set or not, none now and then) with a train
# of six pieces (indexes, flags 0x40 / 0x80, neighbours, definitions with flag 2, tiles and types), the camera's turn, the focus point.
# usage: editupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, fill, log_region, put, set_float_return, set_return, strings  # noqa: E402

HOOKS = {0x443ee0: _recorder(1, 1, cdecl=True), 0x444040: _recorder(2, 0, cdecl=True), 0x4346c0: _recorder(3, 2, cdecl=True),
         0x444090: _recorder(4, 2, cdecl=True), 0x433f70: _recorder(5, 3, cdecl=True), 0x442f40: _recorder(6, 9, cdecl=True),
         0x443df0: _recorder(7, 7, cdecl=True), 0x434740: _recorder(8, 7, cdecl=True), 0x443f40: _recorder(9, 5, cdecl=True),
         0x447830: _recorder(10, 7, cdecl=True)}
B = Bench([(0x443340, 0x443deb), (0x46f220, 0x46f22c), (0x43e160, 0x43e1c0), (0x52fba0, 0x52fbb3)], hooks=HOOKS)
LINK, PIECES, DEFS, TYPES, FOUND, VECS = RAM, RAM + 0x1000, RAM + 0x2000, RAM + 0x3000, RAM + 0x3100, RAM + 0x3200
N = 6
SINES = struct.pack('<fII', 4096 / (2 * math.pi), 0xfff, 0) + b''.join(struct.pack('<f', math.sin(2 * math.pi * i / 4096)) for i in range(4096))


def value(r, scale=None):
    return r.f(scale) if r.random() < 0.3 else fb(r.choice([0.0, -0.0, 1.0, 0.5, -2.0, 3.25, r.uniform(-20, 20), r.uniform(-1, 1)]))


def piece(i):
    return PIECES + 0x200 * i


def train(r):
    pieces, defs, types = bytearray(0x200 * N), bytearray(0x40 * N), bytearray(0x100)
    put(types, 0, 'I', 5)
    put(types, 0xcc, 'I', r.choice([0, 3, 8, 0xffffffff]))
    for i in range(N):
        p = 0x200 * i
        put(pieces, p, 'I', r.choice([i, i, i + 1, 0, r.randrange(8)]))
        put(pieces, p + 8, 'I', r.choice([0, 0x40, 0xc0, 0x80, 0x4000, 0x40 | 0x4000, r.getrandbits(32)]))
        put(pieces, p + 0x64, 'I', piece(i - 1) if i and r.random() < 0.9 else 0)
        put(pieces, p + 0x68, 'I', piece(i + 1) if i < N - 1 and r.random() < 0.9 else 0)
        put(pieces, p + 0xe0, 'I', value(r))
        put(pieces, p + 0x100, 'I', DEFS + 0x40 * i)
        d = 0x40 * i
        defs[d + 4] = r.choice([0, 0, 2, 1, 3, r.getrandbits(8)])
        put(defs, d + 0xc, '2i', r.randrange(-2, 6), r.randrange(-2, 6))
        put(defs, d + 0x14, 'II', r.choice([1, 2]), r.choice([TYPES, TYPES, TYPES + 0x80]))
    return pieces, defs, types


def case(r):
    log = log_region()
    pieces, defs, types = train(r)
    link = bytearray(0x60)
    put(link, 0x40, 'I', 0 if r.random() < 0.05 else r.choice([1, 0x80000000, r.getrandbits(32)]))
    put(link, 0x58, 'I', piece(r.randrange(N)))
    edit = bytearray(0x100)
    for k in range(0, 0x100, 4):
        put(edit, k, 'I', value(r))
    mode = r.deal(['adjust', 'vector', 'span', 'move', 'mixed', 'adjust'])
    flags = r.choice([1, 2, 4, 8, 0x10, 0x20, 0x3f, 0, 2, 4])
    flags |= {'adjust': r.choice([0x200, 0x400, 0x600]), 'vector': r.choice([0x800, 0x1000, 0x1800]), 'span': 0x80 | r.choice([0, 0x100]),
              'move': 0x100, 'mixed': r.getrandbits(16) & 0x1f80}[mode]
    if r.random() < 0.4:
        flags |= 0x2000
    if r.random() < 0.05:
        flags = r.getrandbits(32)
    put(edit, 0x58, 'I', flags)
    world = bytearray(0xd0)   # 0x790a00: the camera's turn +0x38, the mouse's start +0xc8 / +0xcc
    put(world, 0x38, 'f', r.uniform(-7, 7))
    if r.random() < 0.2:
        put(world, 0x38, 'I', value(r))
    put(world, 0xc8, 'II', value(r), value(r))
    for at, start, last in ((0x30, 0xc8, 0x68), (0x34, 0xcc, 0x18)):   # (the mouse unmoved, its last move 0, now and then)
        if r.random() < 0.35:
            put(edit, at, '4s', bytes(world[start:start + 4]))
            put(edit, last, 'f', r.choice([0.0, -0.0]))
    for at in (0xec, 0xa8):   # (steps)
        put(edit, at, 'f', r.choice([1.0, 0.5, 0.01, -0.25, 4.0]))
    for lo, hi in ((0x1c, 0x5c), (0x20, 0x54), (0xe4, 0x6c), (0xe8, 0x70)):   # (the ranges about the values)
        if r.random() < 0.8:
            a, b = sorted([r.uniform(-3, 3), r.uniform(-3, 3)])
            put(edit, lo, 'f', a)
            put(edit, hi, 'f', b)
    for at in (0xd4, 0xd0, 0xf0, 0xf4):
        if r.random() < 0.7:
            put(edit, at, 'f', r.uniform(-2, 2))
    vecs = bytearray(0x40)
    put(edit, 0xfc, 'I', VECS)
    put(edit, 0x14, 'I', VECS + 0x20)
    put(edit, 0xf8, 'I', VECS + 0x10 if r.random() < 0.7 else 0)
    put(edit, 0x10, 'I', VECS + 0x30 if r.random() < 0.7 else 0)
    car = piece(r.randrange(N)) if r.random() < 0.8 else 0
    put(edit, 0x40, 'I', car)
    ends = [piece(i) for i in range(N)]
    span = r.choice(ends) if r.random() < 0.97 else 0
    put(edit, 0xdc, 'I', span)
    put(edit, 0xa4, 'I', r.choice(ends) if r.random() < 0.97 else 0)
    put(edit, 0x9c, 'i', r.choice([0, 5, -5, 100, -0x7fffffff, r.getrandbits(31)]))
    put(edit, 0x98, 'f', r.choice([0.0, 2.5, -1.5, 40.75, r.uniform(-100, 100)]))
    put(edit, 0x24, 'I', r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    ride = r.choice([0, 1, 5, 9, r.getrandbits(32)])
    put(edit, 0xac, 'I', ride if r.random() < 0.3 else r.choice([0, 1, 5]))
    fx, fz = r.randrange(-2, 6), r.randrange(-2, 6)
    focus = struct.pack('<3f', fx * 10 + r.uniform(0, 9.99) if fx >= 0 else fx * 10 - r.uniform(0, 9.99), r.uniform(-5, 5),
                        fz * 10 + r.uniform(0, 9.99) if fz >= 0 else fz * 10 - r.uniform(0, 9.99))
    if r.random() < 0.1:
        focus = struct.pack('<3I', value(r), value(r), value(r))
    sel = struct.unpack_from('<I', edit, 0xa4)[0]
    if sel and r.random() < 0.4:   # (the focus on the moved piece's own tile)
        d = 0x40 * ((sel - PIECES) // 0x200)
        put(defs, d + 0xc, '2i', fx, fz)
    found = bytearray(0x40)
    put(found, 0x14, 'II', r.choice([1, 2]), r.choice([TYPES, TYPES, TYPES + 0x80]))
    put(found, 0x30, 'I', r.choice([0, 2, 7, 9, 0xffffffff]))
    set_return(log, 1, r.choice([0, span, span, car] + ends))
    set_return(log, 5, r.choice([0, FOUND, FOUND]))
    set_return(log, 8, r.choice([0, 1]))
    set_float_return(log, 10, r.choice([0.0, 12.5, -3.25, 100.0, 7.0, r.uniform(-50, 50)]))
    strings(log, 6, 0, out=1 << 5 | 1 << 6)
    fill(log, 6, 5, struct.pack('<f', r.uniform(-20, 20)), which=0)
    fill(log, 6, 6, struct.pack('<f', r.uniform(-20, 20)), which=1)
    strings(log, 7, 0, out=1 << 4)
    blob(log, 7, 3, 1)
    regions = {LOG: log, LINK: link, PIECES: pieces, DEFS: defs, TYPES: types, FOUND: found, VECS: vecs,
               0x790fec: struct.pack('<I', LINK if r.random() < 0.97 else 0), 0x79c600: edit, 0x790a00: world,
               0x7a1c70: focus, 0x7a1cd4: struct.pack('<I', ride), 0x785a2c: struct.pack('<I', r.choice([0, 0, 0, 1])),
               0x81af4c: struct.pack('<I', r.getrandbits(32)), 0x7b095c: SINES}
    return Call(0x443340, [], regions)


if __name__ == '__main__':
    B.main(case)
