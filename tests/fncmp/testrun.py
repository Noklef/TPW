# The coaster test run (src/game/coaster/testrun: Coaster_TestRun 0x43c450) against the original. Its calls are cdecl recorders (the
# driver's TestRunEnv): CoasterRating_Reset 0x43dbf0 (1), CoasterRating_Step 0x43dc50 (2; its force logged as the three floats, too),
# CoasterRating_Finish 0x43df90 (3), Coaster_SetTrainCount 0x43ba20 (4), CoasterCar_Wake 0x43be10 (5; the train's flags logged), Obj_RemoveFromRect 0x45be80 (6),
# Holder_Sleep 0x45bdd0 (7), CoasterCar_StopSounds 0x439670 (8) and Coaster_SpaceCars 0x43c1f0 (9). Vec3_AngleZ runs as it is.
# The world: the sine table, a coaster with a type (flags 0x100 and 8, the speed's factors, the up vector, the trains' room and gap), a
# track of 2 to 11 nodes, each with a piece and its kind, four trains with cars (models, strip objects), the coaster's flags 0x1000,
# 0x4000 and 0x800. The log holds 40 calls, so the track stays short.
# usage: testrun.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, blob, fb, log_region, put, set_return  # noqa: E402

HOOKS = {0x43dbf0: _recorder(1, 1, cdecl=True), 0x43dc50: _recorder(2, 7, cdecl=True), 0x43df90: _recorder(3, 4, cdecl=True),
         0x43ba20: _recorder(4, 2, cdecl=True), 0x43be10: _recorder(5, 2, cdecl=True), 0x45be80: _recorder(6, 3, cdecl=True),
         0x45bdd0: _recorder(7, 1, cdecl=True), 0x439670: _recorder(8, 2, cdecl=True), 0x43c1f0: _recorder(9, 1, cdecl=True)}
B = Bench([(0x43c450, 0x43cd55), (0x46f230, 0x46f23e)], hooks=HOOKS)
COASTER, TYPE, LEADER, NODES, PIECES, KINDS = RAM, RAM + 0x400, RAM + 0x800, RAM + 0x1000, RAM + 0x2000, RAM + 0x2400
TRAINS, CARS, MODELS, STRIPS = RAM + 0x2800, RAM + 0x3000, RAM + 0x4000, RAM + 0x4400
N_TRAINS, N_CARS, N_MODELS, N_STRIPS = 4, 6, 6, 3
SINES = struct.pack('<fII', 4096 / (2 * math.pi), 0xfff, 0) + b''.join(struct.pack('<f', math.sin(2 * math.pi * i / 4096)) for i in range(4096))


def value(r, scale=None):
    return r.f(scale) if r.random() < 0.3 else fb(r.choice([0.0, -0.0, 1.0, 0.5, -2.0, 3.25, r.uniform(-20, 20), r.uniform(-1, 1)]))


def track(r, n):
    nodes, pieces, kinds = bytearray(0x38 * (n + 2)), bytearray(0x20 * n), bytearray(0x10 * n)
    x, y, z = [r.uniform(-30, 30) for _ in range(3)]
    for i in range(n + 2):
        p = 0x38 * i
        x, y, z = x + r.uniform(-8, 8), y + r.choice([0.0, r.uniform(-4, 4), r.uniform(-0.02, 0.02)]), z + r.uniform(-8, 8)
        put(nodes, p, 'I', r.choice([0, 8, 1, 0xffffffff, r.getrandbits(32)]))
        put(nodes, p + 4, '3f', x, y, z)
        if r.random() < 0.08:
            put(nodes, p + 4 + 4 * r.randrange(3), 'I', r.f())
        put(nodes, p + 0x10, '3f', r.uniform(-7, 7), r.uniform(-7, 7), r.uniform(-7, 7))
        if r.random() < 0.08:
            put(nodes, p + 0x10 + 4 * r.randrange(3), 'I', r.f())
        put(nodes, p + 0x1c, 'I', value(r))
        put(nodes, p + 0x2c, 'I', PIECES + 0x20 * (i % n))
    if r.random() < 0.03:   # (a NaN height: fcom's unordered)
        put(nodes, 0x38 * r.randrange(n) + 8, 'I', 0x7fc00000)
    if r.random() < 0.15:   # (a climb of just the type's 0.01745)
        i = r.randrange(n)
        climb = 0.01745 / math.sqrt(1 - 0.01745 * 0.01745) * (1 + r.choice([0, 0, 1e-7, -1e-7]))
        put(nodes, 0x38 * i + 4, '3f', 0.0, 0.0, 0.0)
        put(nodes, 0x38 * (i + 1) + 4, '3f', 0.0, climb, 1.0)
    for i in range(n):
        put(pieces, 0x20 * i + 0x10, 'I', KINDS + 0x10 * i)
        put(kinds, 0x10 * i, 'II', r.getrandbits(32), r.getrandbits(32))
        put(kinds, 0x10 * i + 8, 'I', r.choice([0, 0x800, r.getrandbits(32)]))
    return nodes, pieces, kinds


def trains(r, n):
    block, cars = bytearray(0x80 * N_TRAINS), bytearray(0x60 * N_CARS * N_TRAINS)
    for t in range(N_TRAINS):
        k = 0x80 * t
        count = r.choice([0, 1, 2, 3, 3, 4])
        first = r.randrange(N_CARS - count + 1)
        put(block, k, 'I', r.choice([0, 1, 5, 2, 3, 4, 7, r.getrandbits(32)]))
        put(block, k + 8, 'II', first, count)
        put(block, k + 0x28, 'II', CARS + 0x60 * N_CARS * t, r.randrange(N_CARS))
        put(block, k + 0x30, 'f', r.uniform(0, n - 1.5))
        if r.random() < 0.05:
            put(block, k + 0x30, 'f', r.choice([-0.0, 0.0, -0.75, n - 1.5, float(n // 2)]))
        for c in range(N_CARS):
            p = 0x60 * (N_CARS * t + c)
            put(cars, p, 'I', r.choice([0, 1, 3, r.getrandbits(32)]))
            put(cars, p + 4, 'I', MODELS + 0x40 * r.randrange(N_MODELS))
            put(cars, p + 0xc, 'I', r.choice([0, 1, 2, 3]))
            put(cars, p + 0x38, 'I', value(r))
    return block, cars


def case(r):
    log = log_region()
    n = r.deal([2, 3, 4, 5, 6, 7, 9, 11])
    nodes, pieces, kinds = track(r, n)
    block, cars = trains(r, n)
    coaster, kind, leader = bytearray(0x140), r.deal([0, 8, 0x100, 0x108]), bytearray(0x100)
    type_ = bytearray(0x400)
    put(type_, 0, 'I', kind | (r.getrandbits(32) & ~0x108))
    put(type_, 0x24, '3f', r.choice([1.0, 0.5, 9.8, r.uniform(0, 12)]), r.uniform(-2, 2), r.uniform(-2, 2))
    put(type_, 0x44, 'f', r.choice([0.0, -0.0, 0.5, 1.0, r.uniform(0.01, 5)]))
    put(type_, 0x54, 'f', r.choice([1.0, 1.5, 0.5, 0.0, r.uniform(0, 4)]))
    put(type_, 0x60, '3f', *(r.choice([0.0, 1.0, r.uniform(-2, 2)]) for _ in range(3)))
    put(type_, 0x6c, 'f', r.choice([0.0, 1.0, 0.1, 0.25, r.uniform(0, 2), -0.5]))
    for at, scale in ((0x24, 12), (0x28, 2), (0x2c, 2), (0x44, 5), (0x54, 4), (0x60, 2), (0x64, 2), (0x68, 2), (0x6c, 2)):
        if r.random() < 0.05:
            put(type_, at, 'I', r.f(scale))
    for at in (0x28, 0x2c, 0x54, 0x60):   # (an infinite or NaN factor: level track, fcom's unordered)
        if r.random() < 0.08:
            put(type_, at, 'I', r.choice([0x7f800000, 0x7fc00000]))
    if r.random() < 0.05:   # (past a float's range in the register)
        put(type_, 0x24, 'f', 1e38)
        put(type_, 0x54, 'f', 0.01)
    room =r.choice([0, 1, 2, 3, 4, 4])
    put(type_, 0x2e8, 'II', room, r.choice([0, 1, 100, 500, 1000, 2500, 30000, r.getrandbits(32)]))
    put(coaster, 0, 'I', r.deal([0, 0x1000, 0x1800, 0x1000, 0x1800, 0x5000, 0x4000, 0x800]) | (r.getrandbits(32) & ~0x5800))
    put(coaster, 4, 'I', TYPE)
    put(coaster, 0x34, 'f', r.choice([1e6, 2.0, 5.0, r.uniform(0, 30), 0.0, math.inf]))
    put(coaster, 0x58, 'I', LEADER)
    put(leader, 0xfc, 'I', r.randrange(1, n + 1))
    put(coaster, 0x9c, 'I', NODES)
    put(coaster, 0xa4, 'I', n)
    put(coaster, 0xa8, 'f', r.choice([float(n), n + r.uniform(0, 4), r.uniform(-2, 20), 0.0, math.nan]))
    put(coaster, 0xac, 'f', r.choice([1.0, 3.0, 10.0, 0.5, r.uniform(0.1, 20)]))
    if r.random() < 0.05:
        wild = r.choice([0x34, 0xa8, 0xac])
        put(coaster, wild, 'I', r.f(30))
        if wild == 0xac:   # (no cars' places with it, as with the lengths)
            put(coaster, 0, 'I', struct.unpack_from('<I', coaster, 0)[0] & ~0x1000)
    put(coaster, 0xf4, 'I', r.randrange(0, room + 1) if r.random() < 0.9 else room)
    put(coaster, 0x128, 'I', TRAINS)
    models = bytearray(0x40 * N_MODELS)
    for m in range(N_MODELS):   # (lengths in reason: a wild one puts a car's node anywhere, and what is mapped there differs between the sides)
        put(models, 0x40 * m + 0x24, '2f', r.uniform(-1, 3), r.uniform(-1, 3))
    table, strips = bytearray(4 * (N_STRIPS + 1)), bytearray(0xd0 * N_STRIPS)
    for s in range(N_STRIPS):
        put(table, 4 * (s + 1), 'I', STRIPS + 0xd0 * s)
        put(strips, 0xd0 * s + 4, 'II', r.choice([0, 0x4000, r.getrandbits(32)]), r.getrandbits(32))
        put(strips, 0xd0 * s + 0xc0, 'II', r.getrandbits(32), r.getrandbits(32))
    blob(log, 2, 5, 7)
    blob(log, 5, 1, 1)   # (the woken train's flags)
    set_return(log, 6, r.getrandbits(32))
    regions = {LOG: log, COASTER: coaster, TYPE: type_, LEADER: leader, NODES: nodes, PIECES: pieces, KINDS: kinds, TRAINS: block, CARS: cars,
               MODELS: models, STRIPS: strips, 0x7a4610: table, 0x7b095c: SINES}
    return Call(0x43c450, [COASTER], regions)


if __name__ == '__main__':
    B.main(case)
