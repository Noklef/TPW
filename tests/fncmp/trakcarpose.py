# A track ride's cars set on the ground (src/game/world/trakcarpose: TrakRide_PoseCars 0x546280) against the original. Terrain_RideAt, Model_BeginView, View_FloorAt and Particle_Spawn are cdecl
# recorders (the driver's TrakPoseEnv): the cells from a table, the floors from a fill. Matrix_ResetRotation, Matrix_RotateEulerFast and __chkstk run as they are. The fixture: 1-6 used cars of
# a pool of 6, at most 4 on the ride (the log keeps 40 calls: each car makes 8 and a spawn), three cells and none, some floors -100 or a NaN, all the flag bits.
# usage: trakcarpose.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, answers, fill, log_region, put, strings  # noqa: E402

HOOKS = {0x450ac0: _recorder(1, 2, cdecl=True), 0x450ea0: _recorder(2, 1, cdecl=True), 0x4511a0: _recorder(3, 3, cdecl=True), 0x521e60: _recorder(4, 4, cdecl=True)}
B = Bench([(0x546280, 0x546c80), (0x46f150, 0x46f870), (0x67a830, 0x67a860), (0x67b110, 0x67b13f)], sines=256, hooks=HOOKS)
POOL, RIDES, MODELS, FIRST, SECOND, THIRD, CELLS, ANSWERS = RAM, RAM + 0x500, RAM + 0x800, RAM + 0xa00, RAM + 0xa40, RAM + 0xa80, RAM + 0xd00, RAM + 0xd40
POOL_SIZE = 6
NAN, SNAN = 0x7fc12345, 0x7f812345


def fbits(x):
    return struct.unpack('<I', struct.pack('<f', x))[0]


def height(r):
    return r.choice([fbits(r.uniform(-300, 300)), fbits(r.uniform(-300, 300)), fbits(r.uniform(-300, 300)), fbits(r.uniform(0, 5)), fbits(0.0), NAN, SNAN, 0xffc00000])


def case(r):
    log = log_region()
    pool = bytearray(r.bytes(0xac * POOL_SIZE))
    used = sorted(r.sample(range(POOL_SIZE), r.randrange(1, POOL_SIZE + 1)))
    count = r.randrange(1, len(used) + 1) if r.random() < 0.3 else len(used)
    index = r.randrange(3)
    ride = RIDES + 0xd0 * index
    rides = bytearray(r.bytes(0xd0 * 3))
    for i in range(3):
        put(rides, 0xd0 * i + 8, 'i', r.choice([0x100, 0x200, 0x40, 0, -0x80, r.randrange(0, 0x1000), r.getrandbits(32) - (1 << 31)]))
    ours = r.sample(used[:count], min(count, r.randrange(1, 5)))
    for i in range(POOL_SIZE):
        at = 0xac * i
        flags = struct.unpack_from('<I', pool, at)[0] & ~1 | (1 if i in used else 0)
        put(pool, at, 'I', flags)
        put(pool, at + 8, '2I', r.randrange(4), r.randrange(4))
        put(pool, at + 0x34, '2i', r.randrange(-0x40000, 0x40000), r.randrange(-0x40000, 0x40000))
        put(pool, at + 0x3c, '2i', r.randrange(-0x4000, 0x4000), r.randrange(-0x4000, 0x4000))
        put(pool, at + 0x4c, 'i', r.choice([0, 0x10, r.randrange(-0x4000, 0x4000), r.getrandbits(32) - (1 << 31)]))
        put(pool, at + 0x5c, '2i', r.randrange(-0x4000, 0x4000), r.randrange(-0x4000, 0x4000))
        put(pool, at + 0x9c, 'I', ride if i in ours else RIDES + 0xd0 * ((index + 1) % 3))
        put(pool, at + 0xa0, 'I', height(r))
    cells = [0, CELLS, CELLS + 0x10, CELLS + 0x20]
    table = struct.pack('<40I', *[r.choice(cells[:r.choice([2, 4])]) for _ in range(40)]) if r.random() < 0.8 else struct.pack('<40I', *[r.choice(cells[1:]) for _ in range(40)])
    answers(log, 1, ANSWERS)
    floors = [r.choice([fbits(-100.0), height(r), height(r), height(r)]) for _ in range(28)]
    fill(log, 3, 2, struct.pack('<28I', *floors), records=28)
    strings(log, 3, 0, out=0b100)
    a = r.choice([fbits(r.uniform(-1, 1)), fbits(r.uniform(-1, 1)), fbits(r.uniform(-3, 3)), fbits(1.0), fbits(-1.0), fbits(0.0), 0x80000000, NAN, SNAN])
    rid = struct.unpack_from('<I', rides, 0xd0 * index)[0]
    handle = (index | rid << 8) & 0xffffffff
    if r.random() < 0.04:
        handle ^= 1 << r.randrange(32)
    loaded = r.random() < 0.97
    regions = {LOG: log, POOL: pool, RIDES: rides, MODELS: r.bytes(0x180), CELLS: r.bytes(0x30), ANSWERS: table,
               FIRST: struct.pack('<16I', *[w for i in range(4) for w in (0, 0, SECOND + 0x10 * i, 0)]),
               SECOND: struct.pack('<16I', *[w for i in range(4) for w in (0, THIRD + 0x80 * i, 0, 0)]),
               THIRD: b''.join(bytes(0x78) + struct.pack('<2I', MODELS + 0x60 * i, 0) for i in range(4)),
               0x7a4610: struct.pack('<4I', *[FIRST + 0x10 * i for i in range(4)]),
               0x877b58: struct.pack('<6I', 0x4a454647 if loaded else r.getrandbits(32), r.getrandbits(32), RIDES, r.getrandbits(32), POOL, count if r.random() < 0.98 else 0),
               0x877358: struct.pack('<512i', *[r.randrange(-0x400, 0x401) if r.random() < 0.9 else r.getrandbits(32) - (1 << 31) for _ in range(512)]),
               0x764cd8: struct.pack('<f', r.choice([49143.0, 49143.0, 49143.0, 1024.0, 0.0]))}
    return Call(0x546280, [handle, a], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
