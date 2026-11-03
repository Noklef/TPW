# The flying rides' step (src/game/world/flyingupdate: FlyingRides_Update 0x55abf0) against the original. Its calls are cdecl recorders (the driver's
# FlyingUpdateEnv): FlyingRide_Launch 0x55a720 (1 of 1), FlyingRide_PlaySlotMotion 0x55aa80 (2 of 5), Rides_GetLinkedElement 0x55a3e0 (3 of 2), Sound_SetParam
# 0x51bc40 (4 of 3), Sound_SetPosition 0x51c270 (5 of 4), Terrain_SurfaceHeight 0x452ae0 (6 of 3: a float), FlyingRide_NextCameraCorner 0x55c270 (7 of 1),
# FlyingRide_PickWaypoint 0x55c350 (8 of 1), FlyingRide_ClearSlot 0x55cda0 (9 of 2). World_Rand runs as it is. The fixture: the table with one or two records,
# each with 1-6 slots in use (every state, one aimed for now and then), near their targets or each other now and then, the playfield's bounds about them.
# usage: flyingupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_float_return, set_return  # noqa: E402

HOOKS = {0x55a720: _recorder(1, 1, cdecl=True), 0x55aa80: _recorder(2, 5, cdecl=True), 0x55a3e0: _recorder(3, 2, cdecl=True), 0x51bc40: _recorder(4, 3, cdecl=True),
         0x51c270: _recorder(5, 4, cdecl=True), 0x452ae0: _recorder(6, 3, cdecl=True), 0x55c270: _recorder(7, 1, cdecl=True), 0x55c350: _recorder(8, 1, cdecl=True),
         0x55cda0: _recorder(9, 2, cdecl=True)}
B = Bench([(0x55abf0, 0x55c240), (0x516330, 0x516370), (0x67a830, 0x67a860)], hooks=HOOKS)
RECORDS, STATE, SINES = RAM, RAM + 0x3000, RAM + 0x4000
SIZE, SLOT = 0x5c + 20 * 0xe4, 0xe4
TABLE = struct.pack('<4096i', *[round(1024 * math.sin(2 * math.pi * j / 4096)) for j in range(4096)])
STATES = list(range(1, 10))


def place(r, near, far=120000):
    k = r.random()
    if k < 0.6:
        return near + r.randrange(-3000, 3000)
    if k < 0.97:
        return r.randrange(-far, far)
    return r.getrandbits(32) - (1 << 31)


def fill_slot(r, rec, s, state, cx, cz):
    base = 0x5c + SLOT * s
    x, y, z = place(r, cx), r.randrange(-1000, 40000), place(r, cz)
    put(rec, base, '7i', r.randrange(100), x, y, z, r.randrange(-0x2000, 0x2000), r.randrange(-500, 500), r.randrange(-0x3000, 0x3000))
    k = r.random()
    tx, tz = (x + r.randrange(-4000, 4000), z + r.randrange(-4000, 4000)) if k < 0.5 else (x + r.randrange(-200000, 200000), z + r.randrange(-200000, 200000))
    put(rec, base + 0x4c, '3I', tx & 0xffffffff, r.choice([y + 10000, y - 10000, y + 10001, r.randrange(0, 50000), r.randrange(0, 50000)]) & 0xffffffff, tz & 0xffffffff)
    put(rec, base + 0x70, '4i', r.randrange(-0x800, 0x800), r.randrange(-0x400, 0x400), r.randrange(-2000, 2000), r.randrange(-500, 500))
    put(rec, base + 0x80, 'I', state)
    put(rec, base + 0x84, 'i', r.choice([0, 1, 2, r.randrange(-5, 300)]))
    put(rec, base + 0x8c, 'i', r.choice([0, 0, 1, -1, r.randrange(-5, 6)]))
    put(rec, base + 0xb4, '3i', r.choice([0, 0, 1]), r.choice([0, 0, 0, 1, 2]), r.randrange(-0x1000, 0x2000))
    put(rec, base + 0xe0, 'I', r.choice([0, RAM + 0x1000 + r.randrange(16), r.getrandbits(32) | 0x80000000]))
    return x, y, z


def record(r):
    rec = bytearray(r.bytes(SIZE))
    hx, hy, hz = r.randrange(-90000, 90000), r.randrange(-500, 12000), r.randrange(-90000, 90000)
    used = r.choice([0, 1, 1, 2, 3, 5])
    put(rec, 0, 'i', r.choice([used, used + 1, max(used - 1, 0), 1, 2]))
    put(rec, 4, 'i', r.randrange(0, 6))
    put(rec, 0xc, 'i', used)
    put(rec, 0x10, 'i', r.randrange(0, 600))
    put(rec, 0x14, '3i', hx, hy, hz)
    put(rec, 0x20, '2i', r.choice([0x400, 0x400, r.randrange(0, 0x1000)]), r.choice([0x400, r.randrange(-0x800, 0x1000)]))
    put(rec, 0x28, '3i', r.choice([0x12c, r.randrange(-300, 1000)]), r.choice([0, 0, 1]), r.choice([0x9c4, 0x1388, r.randrange(-0x1000, 0x8000)]))
    put(rec, 0x34, '4i', r.choice([0, 1, 3]), r.choice([0, 1]), r.choice([0, 0, 1]), r.randrange(-0x2000, 0x2000))
    put(rec, 0x44, 'i', r.choice([0, 0, 1]))
    put(rec, 0x4c, '3i', r.choice([0, 0, 1]), r.choice([5, r.randrange(-20, 40)]), r.randrange(0, 200))
    for s in range(20):
        put(rec, 0x5c + SLOT * s + 0x80, 'I', 0)
    n = r.choice([1, 1, 2, 3, 6])
    slots = r.sample(range(20), n)
    if r.random() < 0.5:
        slots[0] = 0
    state = r.deal(STATES)
    cx, cz = (hx, hz) if r.random() < 0.5 else (r.randrange(-90000, 90000), r.randrange(-90000, 90000))
    for s in slots:
        st = state if s == slots[0] else r.choice(STATES + [0, 1, 1, 0xa, r.getrandbits(32)])
        fill_slot(r, rec, s, st, cx, cz)
    k = r.random()
    if k < 0.1 and n > 1:
        pair(r, rec, sorted(slots))
    elif k < 0.2 and n == 1:
        exact_roll(r, rec, slots[0])
    return rec, hx, hz


def pair(r, rec, slots):
    """Two close slots whose speeds tie (the first's eased 0x400 is 900, its goal 0)."""
    a, b = 0x5c + SLOT * slots[0], 0x5c + SLOT * slots[1]
    put(rec, a + 0x78, '2i', 0x400, 0)
    put(rec, a + 0x80, 'I', r.choice([2, 4, 5, 8]))
    put(rec, b + 0x80, 'I', r.choice([2, 3, 4, 5, 6, 8, 9]))
    put(rec, 0x20, 'i', 0x400)
    x, y, z = struct.unpack_from('<3i', rec, a + 4)
    put(rec, b + 4, '3i', x + r.randrange(-3000, 3000), y + r.randrange(-1000, 1000), z + r.randrange(-3000, 3000))
    put(rec, b + 0x78, 'i', 900)


def truncdiv(a, b):
    return abs(a) // b * (1 if a >= 0 else -1)


def exact_roll(r, rec, s):
    """A still slot on its target (bearing 0), its roll exactly 0x800 either side of the wanted: the turn and roll worked out here."""
    base = 0x5c + SLOT * s
    wide = struct.unpack_from('<i', rec, 0x24)[0]
    x, z = struct.unpack_from('<i', rec, base + 4)[0], struct.unpack_from('<i', rec, base + 0xc)[0]
    put(rec, base + 0x4c, 'i', x)
    put(rec, base + 0x54, 'i', z)
    put(rec, base + 0x70, '4i', 0, 0, 0, 0)
    put(rec, base + 0x80, 'I', 9)
    for _ in range(60):
        h = r.randrange(0x1000)
        turn = -h & 0xfff
        if turn >= 0x800:
            turn -= 0xfff
        yaw = truncdiv(turn, 2)
        roll = truncdiv((-truncdiv(yaw, 4) - truncdiv(yaw, 2)) * wide, 1024) & 0xfff
        if (roll >= 0x800) == (s & 1 == 0):
            break
    put(rec, base + 0x10, 'i', h)
    put(rec, base + 0x18, 'i', roll - 0x800 if roll >= 0x800 else roll + 0x800)


def case(r):
    log = log_region()
    table = bytearray(400)
    regions = {LOG: log, SINES: TABLE, STATE: struct.pack('<I', r.getrandbits(32)), 0x7cf83c: struct.pack('<I', (STATE - 0x1da708) & 0xffffffff), 0x8791c8: struct.pack('<I', SINES)}
    two = r.random() < 0.2
    rec, hx, hz = record(r)
    regions[RECORDS] = bytes(rec)
    put(table, 4 * r.choice([1, 99, r.randrange(1, 100)]), 'I', RECORDS)
    put(table, 0, 'I', r.getrandbits(32))   # (entry 0: never read)
    if two:
        rec2, _, _ = record(r)
        regions[RECORDS + 0x1400] = bytes(rec2)
        put(table, 4 * r.randrange(1, 100), 'I', RECORDS + 0x1400)
    regions[0x8791f8] = table
    far = r.random() < 0.3
    span = r.choice([20000, 200000, 400000])
    ux, lx = hx + r.randrange(1, span), hx - r.randrange(0, span)
    uz, lz = hz + r.randrange(1, span), hz - r.randrange(0, span)
    if far:
        ux, lx = lx + 1, lx
    regions[0x7660b8] = struct.pack('<4i', ux, lx, uz, lz)
    regions[0x7a0314] = struct.pack('<i', r.choice([0x40, 1, r.randrange(1, 0x80)]))
    regions[0x7a0374] = struct.pack('<i', r.choice([0x40, 1, r.randrange(1, 0x80)]))
    set_float_return(log, 6, r.choice([0.0, 1.5, 10.0, 30.25, r.randrange(0, 100) * 0.5, -3.5]))
    set_return(log, 3, r.getrandbits(32))
    set_return(log, 4, r.getrandbits(32))
    set_return(log, 5, r.getrandbits(32))
    return Call(0x55abf0, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
