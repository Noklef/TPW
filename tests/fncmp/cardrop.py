# A coaster car dropped on a cell (src/game/coaster/train: Coaster_DropCar 0x442f40) against the original. Its calls are recorders (cdecl,
# as the driver's TrainEnv has them): Train_RemoveCar 0x442d70 (9 of 2), CoasterLink_RemoveEntry 0x442ea0 (13 of 3), Map_SetRideCell
# 0x452d30 (14 of 5), Train_SetCouplings 0x432d50 (15 of 2), Train_PlaceCars 0x432df0 (16 of 4), Train_ShrinkToTop 0x433a90 (19 of 3),
# Train_InsertCar 0x433bd0 (20 of 10), Pylon_FindBraceLevel 0x433300 (21 of 5), Animator_PlaceOnGround 0x4671f0 (22 of 7),
# CoasterLink_AddEntry 0x432c10 (23 of 6), Contacts_MarkLostPair 0x4346f0 (24 of 1), Model_FitToGrid 0x469940 (25 of 4); ShrinkToTop and
# FindBraceLevel answer 0 now and then; CoasterEvent_Make and the empty Debug_Printf run. The car (+0x70 a ride of four whose holder's
# +8 -> +4 model has a frame +0x78 -> +0x44) on its train (+4 byte flag 2 or not, +0xc / +0x10 its cell, +0x18 a type (+0xb4 / +0xb8),
# +0x30 one car, a few or any, +0x34 a car); the cell its train's mostly, else about the map (0x7a0314 / 0x7a0374, 0x7a0854 -> +0x6c ->
# +0x18 its width), negative or past it, or any; the pylon none, another train or the car's own; the owner none now and then; 0x79c694 a
# float, 0x79c660 the train or not; outX / outY given or not; noShrink 0, 1 or any.
# usage: cardrop.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fail_after, fill, log_region, put, strings  # noqa: E402

HOOKS = {0x442d70: _recorder(9, 2, cdecl=True), 0x442ea0: _recorder(13, 3, cdecl=True), 0x452d30: _recorder(14, 5, cdecl=True),
         0x432d50: _recorder(15, 2, cdecl=True), 0x432df0: _recorder(16, 4, cdecl=True), 0x433a90: _recorder(19, 3, cdecl=True),
         0x433bd0: _recorder(20, 10, cdecl=True), 0x433300: _recorder(21, 5, cdecl=True), 0x4671f0: _recorder(22, 7, cdecl=True),
         0x432c10: _recorder(23, 6, cdecl=True), 0x4346f0: _recorder(24, 1, cdecl=True), 0x469940: _recorder(25, 4, cdecl=True)}
RANGES = [(0x442f40, 0x443339), (0x432bd0, 0x432c10), (0x5da3c0, 0x5da3d0)]
B = Bench(RANGES, hooks=HOOKS)
OWNER, CARS, TRAINS, TYPE, HOLD, MAP, OUT = (RAM + k * 0x1000 for k in range(1, 8))


def case(r):
    log = log_region()
    strings(log, 21, 0, out=0b1100)   # (Pylon_FindBraceLevel's level and height: stack addresses; it writes them mostly, or leaves them)
    if r.random() < 0.8:
        fill(log, 21, 2, struct.pack('<I', r.choice([0, 1, 2, r.getrandbits(32)])), which=0)
        fill(log, 21, 3, r.floats(bytearray(4), [0]), which=1)
    if r.random() < 0.25:
        fail_after(log, 21, 0, 0)   # (Pylon_FindBraceLevel: no level)
    if r.random() < 0.25:
        fail_after(log, 19, 0, 0)   # (Train_ShrinkToTop: nothing to shrink)
    cars = bytearray(r.bytes(4 * 0x110))
    for k in range(4):
        put(cars, 0x110 * k + 0x70, 'I', r.randrange(4))
        put(cars, 0x110 * k + 0x100, 'I', TRAINS + 0x80 * r.randrange(3))
    car = CARS
    width, depth = r.choice([(128, 128), (r.randrange(1, 200), r.randrange(1, 200))])
    trains = bytearray(r.bytes(3 * 0x80))
    for k in range(3):
        t = 0x80 * k
        trains[t + 4] = r.getrandbits(8) & ~2 | r.choice([0, 0, 2])
        put(trains, t + 0xc, 'ii', r.randrange(-2, width + 2), r.randrange(-2, depth + 2))
        put(trains, t + 0x18, 'I', TYPE)
        put(trains, t + 0x30, 'I', r.choice([1, 1, 2, 3, 0, r.getrandbits(32)]))
        put(trains, t + 0x34, 'I', CARS + 0x110 * r.randrange(4))
    put(cars, 0x100, 'I', TRAINS)
    own = struct.unpack_from('<ii', trains, 0xc)
    if r.random() < 0.35:
        x, y = own
    elif r.random() < 0.9:
        x, y = r.randrange(-3, width + 3), r.randrange(-3, depth + 3)
    else:
        x, y = r.getrandbits(32) - (1 << 31), r.choice([own[1], r.getrandbits(32) - (1 << 31)])
    ttype = bytearray(r.bytes(0x100))
    put(ttype, 0xb4, 'II', r.randrange(1, 5), r.randrange(1, 5))
    hold = bytearray(r.bytes(0x400))
    for k in range(4):   # (holder +8 -> object +4 -> model +0x78 -> frame +0x44)
        o = 0x100 * k
        put(hold, o + 8, 'I', HOLD + o + 0x20)
        put(hold, o + 0x20 + 4, 'I', HOLD + o + 0x40)
        put(hold, o + 0x40 + 0x78, 'I', HOLD + o + 0xb0)
        r.floats(hold, [o + 0xb0 + 0x44])
        if r.random() < 0.1:
            put(hold, o + 0xb0 + 0x44, 'I', 0x7f800001 | r.getrandbits(22) | (r.getrandbits(1) << 31))   # (a signalling NaN)
    mp = bytearray(r.bytes(0x100))
    put(mp, 0x6c, 'I', MAP + 0x80)
    put(mp, 0x80 + 0x18, 'I', r.choice([width, r.randrange(1, 300)]))
    glob = bytearray(4)
    regions = {LOG: log, CARS: cars, TRAINS: trains, TYPE: ttype, HOLD: hold, MAP: mp, OWNER: r.bytes(0x150), OUT: r.bytes(8),
               0x7a4610: struct.pack('<4I', *[HOLD + 0x100 * k for k in range(4)]),
               0x7a0854: struct.pack('<I', MAP), 0x7a0314: struct.pack('<i', width), 0x7a0374: struct.pack('<i', depth),
               0x79c694: r.floats(glob, [0]), 0x79c660: struct.pack('<I', r.choice([TRAINS, 0, TRAINS + 0x80])),
               0x791050: r.bytes(8), 0x791ce8: r.bytes(0x10)}
    pylon = r.choice([0, TRAINS + 0x80, TRAINS + 0x100, TRAINS + 0x80, TRAINS])
    owner = OWNER if r.random() < 0.95 else 0
    outs = [r.choice([OUT, OUT, 0]), r.choice([OUT + 4, OUT + 4, 0])]
    return Call(0x442f40, [owner, car, pylon, x & 0xffffffff, y & 0xffffffff, *outs, r.getrandbits(32),
                           r.choice([0, 0, 1, r.getrandbits(32)])], regions)


if __name__ == '__main__':
    B.main(case)
