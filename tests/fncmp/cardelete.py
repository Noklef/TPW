# A car deleted from a coaster link (src/game/coaster/train: Coaster_DeleteCar 0x444cb0) against the original. Its calls are recorders
# (cdecl, as the driver's TrainEnv has them): Train_RemoveCar 0x442d70 (9 of 2), Highlight_Unflash 0x469650 (10 of 1), Contacts_ClearPair
# 0x43af70 (11 of 2), Car_Free 0x434570 (12 of 2), CoasterLink_RemoveEntry 0x442ea0 (13 of 3), Map_SetRideCell 0x452d30 (14 of 5),
# Train_SetCouplings 0x432d50 (15 of 2), Train_PlaceCars 0x432df0 (16 of 4), CoasterEdit_SetBounds 0x434f10 (17 of 0); Flags_SetChanged
# runs. The owner: +0x3c flag 2 or not, its +0x54 / +0x58 / +0x5c / +0x60 / +0x64 / +0x144 cars of the chain or not; a chain of one to six
# cars (+0x64 next, +0x68 previous; +8 flags 0x40 / 0x80 or not, the first mostly without 0x40; +0x70 a ride of the table 0x7a4610 whose
# holder flashes (+4 0x4000000) or not, its model's (+8) part entries (+0x28 -> +4, 0x14 bytes each) with a +0x34 or none), each on one
# of two trains (+0x100: +4 flag 1 or not, +0x18 a type with a part index +0x2c, +0x30 no cars or some, +0x34 a car); the car deleted
# any of them; the selected car (0x79c640) it, another or none; the current link (0x790fec) none or one whose +4 byte has flag 2 or not.
# FlaggedEnd (Coaster_DeleteFlaggedEnd 0x435530, Coaster_DeleteCar running): the same world, each car its own train, the last few (none,
# one to all) with flag 1, the owner's +0x58 the last car mostly.
# usage: cardelete.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Delete,FlaggedEnd)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put  # noqa: E402

HOOKS = {0x442d70: _recorder(9, 2, cdecl=True), 0x469650: _recorder(10, 1, cdecl=True), 0x43af70: _recorder(11, 2, cdecl=True),
         0x434570: _recorder(12, 2, cdecl=True), 0x442ea0: _recorder(13, 3, cdecl=True), 0x452d30: _recorder(14, 5, cdecl=True),
         0x432d50: _recorder(15, 2, cdecl=True), 0x432df0: _recorder(16, 4, cdecl=True), 0x434f10: _recorder(17, 0, cdecl=True)}
RANGES = [(0x444cb0, 0x444f30), (0x4346c0, 0x434700), (0x5da3c0, 0x5da3d0), (0x435530, 0x43556d)]
B = Bench(RANGES, hooks=HOOKS)
OWNER, TYPE, GAME, CARS, TRAINS, HOLD = RAM + 0x1000, RAM + 0x1200, RAM + 0x1300, RAM + 0x2000, RAM + 0x3000, RAM + 0x4000
NRIDES = 6


def rides(r, regions):
    """The ride table 0x7a4610 (NRIDES holders: +4 flashing or not, +8 a model whose +0x28 -> +4 part entries, 0x14 bytes each, with
    an entry (+4) giving a float +0x34, or none)."""
    hold = bytearray(r.bytes(0x100 * NRIDES))
    for k in range(NRIDES):
        o = 0x100 * k
        put(hold, o + 4, 'I', r.getrandbits(32) & ~0x4000000 | r.choice([0, 0x4000000]))
        put(hold, o + 8, 'I', HOLD + o + 0x20)
        put(hold, o + 0x20 + 0x28, 'I', HOLD + o + 0x60)
        put(hold, o + 0x60 + 4, 'I', HOLD + o + 0x70)
        for e in range(4):
            put(hold, o + 0x70 + 0x14 * e + 4, 'I', r.choice([0, HOLD + o + 0x80]))
        r.floats(hold, [o + 0x80 + 0x34])
    regions[HOLD] = hold
    regions[0x7a4610] = struct.pack('<%dI' % NRIDES, *[HOLD + 0x100 * k for k in range(NRIDES)])


def case(r):
    op = r.deal(OPS)
    regions = {LOG: log_region()}
    rides(r, regions)
    m = r.randrange(1, 7)
    cars = [CARS + 0x110 * i for i in range(m)]
    trains = [TRAINS, TRAINS + 0x80]
    block = bytearray(r.bytes(0x110 * m))
    handle = r.getrandbits(32) if r.random() < 0.2 else r.randrange(0x100)
    for i, c in enumerate(cars):
        o = 0x110 * i
        put(block, o, 'I', handle + i)
        flags = r.getrandbits(32) & ~0xc0 | r.choice([0, 0, 0x40, 0x80, 0xc0])
        if i == 0 and r.random() < 0.9:
            flags &= ~0x40
        put(block, o + 8, 'I', flags)
        put(block, o + 0x64, 'I', cars[i + 1] if i + 1 < m else 0)
        put(block, o + 0x68, 'I', cars[i - 1] if i else 0)
        put(block, o + 0x70, 'I', r.randrange(NRIDES))
        put(block, o + 0x100, 'I', r.choice(trains[:1] * 3 + trains[1:]))
    regions[CARS] = block
    train = bytearray(r.bytes(0x100))
    for t in (0, 0x80):
        train[t + 4] = r.getrandbits(8) & ~1 | r.choice([0, 0, 1])
        put(train, t + 0x18, 'I', TRAINS + 0x100 + t)
        put(train, t + 0x30, 'I', r.choice([0, 1, 2, r.getrandbits(32)]))
        put(train, t + 0x34, 'I', r.choice(cars))
    regions[TRAINS] = train
    ttype = bytearray(r.bytes(0x100))
    for t in (0, 0x80):
        put(ttype, t + 0x2c, 'I', r.randrange(4))
    regions[TRAINS + 0x100] = ttype
    owner = bytearray(r.bytes(0x150))
    put(owner, 4, 'I', TYPE)
    put(owner, 0x3c, 'I', r.getrandbits(32) & ~6 | r.choice([0, 2, 4, 6]))

    def some():
        return r.choice(cars + [0, r.choice(cars)])
    for off in (0x54, 0x58, 0x5c, 0x60, 0x64, 0x144):
        put(owner, off, 'I', some())
    car = r.choice(cars)
    for off in (0x54, 0x58, 0x5c, 0x60, 0x144):
        if r.random() < 0.25:
            put(owner, off, 'I', car)
    regions[OWNER] = owner
    regions[TYPE] = r.bytes(0xb0)
    game = bytearray(r.bytes(8))
    put(game, 4, 'I', GAME + 8)
    regions[GAME] = game + bytes([r.getrandbits(8) & ~2 | r.choice([0, 2, 2])])
    regions[0x790fec] = struct.pack('<I', r.choice([0, GAME, GAME, OWNER]))
    glob = bytearray(r.bytes(0x70))
    put(glob, 0x40, 'I', r.choice([car, car, cars[0], 0]))
    regions[0x79c600] = glob
    if op == 'FlaggedEnd':   # (each car its own train, those of the last few with flag 1; now and then all: the walk reads null)
        own = bytearray(r.bytes(0x80 * m))
        k = r.choice([0, 1, 1, 2, 3, m - 1, m] if r.random() < 0.95 else [m])
        for i in range(m):
            t = 0x80 * i
            own[t + 4] = own[t + 4] & ~1 | (1 if i >= m - k else 0)
            put(own, t + 0x18, 'I', TRAINS + 0x100 + r.choice([0, 0x80]))
            put(own, t + 0x30, 'I', r.choice([0, 1, 2]))
            put(own, t + 0x34, 'I', r.choice(cars))
            put(block, 0x110 * i + 0x100, 'I', TRAINS + 0x200 + t)
        regions[TRAINS + 0x200] = own
        put(owner, 0x58, 'I', cars[-1] if r.random() < 0.9 else r.choice(cars))
        return Call(0x435530, [OWNER], regions, ret='v')
    return Call(0x444cb0, [OWNER, car], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', 'Delete,FlaggedEnd').split(',') if k]

if __name__ == '__main__':
    B.main(case)
