# A track ride's cars added, removed, started and broken (src/game/world/trakridecars: 0x544c80, 0x544f90, 0x549b80, 0x549db0, 0x54ad90) against the original. Their calls are cdecl recorders (the
# driver's TrakRideCarsEnv): TrakCar_ChooseRoute 0x54a040 (1 of 2), Particle_Spawn 0x521e60 (2 of 4), Layer_ClearQueue 0x473270 (3 of 2), Animator_Play 0x4732a0 (4 of 6), TrakRide_Load 0x549c60 (5 of 2),
# Ride_Create 0x463060 (6 of 10), TrakCar_Remove 0x54ae50 (7 of 1); IntArea_Contains, ObjectDefs_Find run as they are. The fixture: the module loaded or not, two rides of each kind and state, a pool of
# 8 cars (some free, some on the other ride), the handle right or stale.
# usage: trakridecars.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

HOOKS = {0x54a040: _recorder(1, 2, cdecl=True), 0x521e60: _recorder(2, 4, cdecl=True), 0x473270: _recorder(3, 2, cdecl=True), 0x4732a0: _recorder(4, 6, cdecl=True), 0x549c60: _recorder(5, 2, cdecl=True),
         0x463060: _recorder(6, 10, cdecl=True), 0x54ae50: _recorder(7, 1, cdecl=True)}
B = Bench([(0x544c80, 0x544e43), (0x544f90, 0x5450d3), (0x549b80, 0x549c5d), (0x549db0, 0x54a03b), (0x54ad90, 0x54ae4b), (0x5493f0, 0x5494c6), (0x549020, 0x54906e), (0x549b50, 0x549b80), (0x412e90, 0x412eea),
           (0x5da3c0, 0x5da3c1), (0x67a830, 0x67a860)], hooks=HOOKS)
RIDES, AREAS, INSTANCE, HOLDER, STATES, ENTRIES, RECORD, DEF, POOL = RAM, RAM + 0x200, RAM + 0x280, RAM + 0x290, RAM + 0x2c0, RAM + 0x2d0, RAM + 0x310, RAM + 0x400, RAM + 0x1000
KINDS = [-14, -11, -10, -9, -7, -6, -4, -3, -1, -1, -2, -15, -13, -5, 0, 1]
OPS = [(0x544c80, 1), (0x544f90, 1), (0x549b80, 1), (0x549db0, 2), (0x54ad90, 2)]
ADDRS = [0, 1, 2, 3, 4, 2, 4, 3]


def case(r):
    log = log_region()
    op = r.deal(ADDRS)
    addr, n = OPS[op]
    index = r.randrange(2)
    rides = bytearray(r.bytes(0xd0 * 2))
    for j in range(2):
        at = 0xd0 * j
        put(rides, at, 'i', r.deal(KINDS))
        put(rides, at + 4, 'i', r.randrange(-1, 300))
        put(rides, at + 8, 'I', r.getrandbits(32))
        put(rides, at + 0x10, 'H', 3)
        put(rides, at + 0x14, '2i', r.randrange(0, 4), r.choice([1, 2, 3, -2]))
        put(rides, at + 0x50, 'I', r.choice([0, 1, 1, 1, 2, 3]))
        put(rides, at + 0x58, 'I', r.choice([0, 0, r.getrandbits(32)]))
        put(rides, at + 0x5c, 'i', r.choice([-3, 0, 1, 5, 0x3f, 0x40, 0x41]))
        put(rides, at + 0x64, 'i', r.choice([0, 1, 6, 0x40, 0x41, 0x50]))
        put(rides, at + 0x74, '2i', r.randrange(-3000, 3000), r.randrange(-3000, 3000))
        put(rides, at + 0xc4, 'I', r.choice([0, r.getrandbits(32)]))
    if r.random() < 0.6:
        put(rides, 0xd0 * index + 0x64, 'i', 0x41)
        put(rides, 0xd0 * index + 0x5c, 'i', r.randrange(0, 0x40))
        put(rides, 0xd0 * index + 0x50, 'I', r.choice([1, 2]))
    serial = struct.unpack_from('<I', rides, 0xd0 * index)[0]
    good = (index | serial << 8) & 0xffffffff
    ride = RIDES + 0xd0 * index
    pool = bytearray(r.bytes(0xac * 8)) + bytes(0xac)     # (a ninth, free: TrakPool_FirstFree's answer when the others are used)
    used = 0
    for k in range(8):
        at = 0xac * k
        on = r.random() < 0.8
        put(pool, at, 'I', (r.getrandbits(32) | 1) if on else (r.getrandbits(32) & ~1))
        used += on
        put(pool, at + 8, 'I', r.choice([0, 1, 2, 3]))
        put(pool, at + 0x24, 'i', r.choice([-1, 0, 1, 2]))
        put(pool, at + 0x2c, 'i', r.choice([-1, -1, 5]))
        put(pool, at + 0x30, 'I', r.choice([0, 0, r.getrandbits(32)]))
        put(pool, at + 0x9c, 'I', r.choice([ride, ride, ride, RIDES + 0xd0 * (1 - index)]))
    count = r.choice([used, used, used, max(used - 1, 0), 0])
    if r.random() < 0.05 and addr == 0x549db0:   # (a full pool: the 0x100 records all used)
        pool = bytearray(b'\x01\x00\x00\x00' + bytes(0xa8)) * 0x100
        count = 0x100
    areas = bytearray()
    in_use = 0
    place_x, place_y = struct.unpack_from('<2i', rides, 0xd0 * index + 0x74)
    for k in range(4):
        fl = r.choice([1, 1, 0x41, 0]) | r.choice([0, 6, 0x18, 0x1e, 0x3e])
        in_use += fl & 1
        areas += struct.pack('<5i', fl, place_x + r.randrange(-300, 300), place_y + r.randrange(-300, 300), r.randrange(10, 400), r.randrange(10, 400)) + bytes(0xc)
    # The instance's +8 -> holder, its +0x28 -> states, its +4 -> entries (each 0x14), an entry's +4 -> a record or none with its place at +0x30.
    instance = struct.pack('<3I', 0, 0, HOLDER)
    holder = bytes(0x28) + struct.pack('<I', STATES)
    states = struct.pack('<2I', 0, ENTRIES)
    entries = bytearray(0x14 * 3)
    for k in range(3):
        put(entries, 0x14 * k + 4, 'I', r.choice([0, RECORD, RECORD, RECORD]))
    record = bytes(0x30) + struct.pack('<3f', *[r.choice([0.0, 1.5, -2.25, 100.0, r.randrange(-500, 500) / 8]) for _ in range(3)])
    defs = bytearray(0x738)
    put(defs, 0, 'I', DEF)
    put(defs, 0x528, 'i', 1)
    put(defs, 0x730, 'H', 0xffff)
    definition = bytearray(r.bytes(0x500))
    put(definition, 0, 'H', 3)
    set_return(log, 6, r.choice([0, 1, 2]))
    set_return(log, 2, r.getrandbits(32))
    loaded = r.random() < 0.95
    handle = r.choice([good, good, good, good, good, good, good ^ 0x100, r.getrandbits(24) << 8 | index])
    if not loaded:
        handle = r.choice([1, good])     # (a handle of 0 faults reading address 0x5c and so on)
    regions = {LOG: log, RIDES: rides, POOL: pool, AREAS: areas, INSTANCE: instance, HOLDER: holder, STATES: states, ENTRIES: bytes(entries), RECORD: record, DEF: definition, 0x7890a0: defs,
               0x7a4610: struct.pack('<4I', *[INSTANCE] * 4), 0x877b58: struct.pack('<I', 0x4a454647 if loaded else 0x12345678), 0x877b60: struct.pack('<I', RIDES), 0x877b68: struct.pack('<2I', POOL, count),
               0x877b70: struct.pack('<2I', 0 if r.random() < 0.1 and addr == 0x549db0 else AREAS, r.choice([in_use, in_use, max(in_use - 1, 0)]))}
    args = [handle] if n == 1 else [handle, r.choice([0, 0, 1, 0x10])]
    return Call(addr, args, regions, ret='v' if addr in (0x544c80, 0x544f90) else 'a')


if __name__ == '__main__':
    B.main(case)
