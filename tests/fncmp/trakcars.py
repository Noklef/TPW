# The track rides' cars' tick (src/game/world/trakcarupdate: TrakCars_Update 0x546c80) against the original. Its calls are cdecl recorders 1-5 of 1 (the driver's TrakCarsEnv; TrakCar_Update's taking one car off now and
# then); IntVec2_Length and World_Rand run as they are. The fixture: 1-5 cars of a pool of 6, close together, the rides' kinds, shares and node chains, the timers about their limits.
# usage: trakcars.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fill, log_region, put  # noqa: E402

HOOKS = {0x5474b0: _recorder(1, 1, cdecl=True), 0x547f50: _recorder(2, 1, cdecl=True), 0x5497b0: _recorder(3, 1, cdecl=True), 0x5494d0: _recorder(4, 1, cdecl=True), 0x547170: _recorder(5, 1, cdecl=True)}
B = Bench([(0x546c80, 0x547135), (0x549020, 0x549070), (0x516330, 0x516370), (0x67a830, 0x67a860)], hooks=HOOKS)
POOL, TYPES, RIDES, NODES, STATE = RAM, RAM + 0x500, RAM + 0x600, RAM + 0x780, RAM + 0x900
KINDS = [-14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -15, 0]
POOL_SIZE = 6


def around(r, spot):
    return spot + r.choice([r.randrange(-20, 20), r.randrange(-0x400, 0x400), r.randrange(-0x4000, 0x4000), 0])


def rides(r):
    """Two rides, each with a chain of nodes (next at +0x14, flags at +0, value +4)."""
    out = bytearray()
    nodes = bytearray()
    for i in range(2):
        n = r.randrange(2, 5)
        base = NODES + 0x60 * i
        special = r.randrange(n)
        for k in range(n):
            flags = r.choice([0, 0, 2, 2]) & ~4 | (4 if k == special else 0)
            node = struct.pack('<II', flags | (r.getrandbits(5) << 3), r.getrandbits(32)) + r.bytes(12) + struct.pack('<I', base + 0x18 * (k + 1) if k + 1 < n else 0)
            nodes += node
        nodes += bytes(0x60 - 0x18 * n)
        if not any(not (struct.unpack_from('<I', nodes, 0x60 * i + 0x18 * k)[0] & 2) for k in range(n)):   # (one without bit 1, or the walk never ends)
            nodes[0x60 * i] &= 0xfd
        ride = bytearray(r.bytes(0xb0))
        put(ride, 0, 'i', r.choice(KINDS))
        put(ride, 0x2c, 'i', r.choice([0x200, 0x400, 0, 0x100, r.randrange(0, 0x400), r.getrandbits(32) - (1 << 31)]))
        put(ride, 0x68, 'I', r.choice([100, 300, 1, 2, 7, r.randrange(1, 5000)]))
        put(ride, 0xa8, 'I', base)
        out += ride
    return out, nodes


def case(r):
    log = log_region()
    pool = bytearray(r.bytes(0xac * POOL_SIZE))
    edge = r.random() < 0.05   # (two cars meeting at a speed about 0x82: a share about the bounce's limit)
    used = sorted(r.sample(range(POOL_SIZE), 2 if edge else r.randrange(1, POOL_SIZE)))
    count = len(used) - 1 if len(used) > 1 and r.random() < 0.3 and not edge else len(used)
    spot = (r.randrange(-30000, 30000), r.randrange(-30000, 30000))
    ride_bytes, nodes = rides(r)
    for i in range(POOL_SIZE):
        at = 0xac * i
        flags = struct.unpack_from('<I', pool, at)[0] & ~1 | (1 if i in used else 0)
        if r.random() < 0.85:
            flags &= ~0x200
        if r.random() < 0.8:
            flags &= ~0x200000
        put(pool, at, 'I', flags)
        put(pool, at + 0x14, 'i', r.randrange(0, 6))
        put(pool, at + 0x34, '2i', around(r, spot[0]), around(r, spot[1]))
        put(pool, at + 0x3c, '2i', r.randrange(-0x1000, 0x1000), r.randrange(-0x1000, 0x1000))
        put(pool, at + 0x44, '2i', r.choice([0, r.randrange(-0x800, 0x800), r.randrange(-0x40000, 0x40000)]), r.choice([0, r.randrange(-0x800, 0x800), r.randrange(-0x40000, 0x40000)]))
        put(pool, at + 0x64, 'i', r.choice([0, 0x10, 0x400, 0x4000, r.randrange(0, 0x8000), 0x10000]))
        put(pool, at + 0x98, '2I', TYPES + 0x40 * r.randrange(2), RIDES + 0xb0 * r.randrange(2))
        put(pool, at + 0xa4, '2i', r.choice([0, -1, 1, 0x10, 0x11, 0x40]), r.choice([0, 0x1f, 0x20, 0x21, 5]))
    types = bytearray(r.bytes(0x80))
    for i in range(2):
        put(types, 0x40 * i, 'B', r.choice([0, 0, 0, 0x7f, 0x80, 0xff]))
        put(types, 0x40 * i + 0x1c, 'I', r.choice([0, NODES + 0x60 * r.randrange(2), NODES + 0x18]))
    sines = [r.randrange(-0x400, 0x401) for _ in range(512)]
    if edge:   # (every sine 0x100 and the second car's whole share: the share is the speed)
        sines = [0x100] * 512
        put(ride_bytes, 0, 'i', r.choice([-10, -9, -7, -4, -14, -11, -6, -3]))
        put(ride_bytes, 0x2c, 'i', 0x400)
        for i, at in enumerate(0xac * k for k in used):
            flags = struct.unpack_from('<I', pool, at)[0] & ~0x200
            put(pool, at, 'I', flags)
            put(pool, at + 0x34, '2i', spot[0] + 8 * i, spot[1])
            put(pool, at + 0x44, '2i', 0, 0)
            put(pool, at + 0x64, 'i', 0x10)
            put(pool, at + 0x98, '2I', TYPES, RIDES)
        put(types, 0, 'B', 0)
        put(pool, 0xac * used[0] + 0x44, 'i', r.choice([0x81, 0x82, 0x82, 0x83]) * r.choice([1, -1]))
    regions = {LOG: log, POOL: pool, TYPES: types, RIDES: ride_bytes, NODES: nodes,
               0x877b58: struct.pack('<2I', 0x4a454647 if r.random() < 0.95 else r.getrandbits(32), r.getrandbits(32)),
               0x877b68: struct.pack('<2I', POOL, count),
               0x877358: struct.pack('<512i', *sines),
               0x7cf83c: struct.pack('<I', (STATE - 0x1da708) & 0xffffffff), STATE: r.bytes(4)}
    if count < len(used):   # (TrakCar_Update takes one car off: the second pass then walks on to the one past the count)
        lows = bytearray(pool[0xac * i] for i in used[:count])
        lows[r.randrange(count)] &= 0xfe
        fill(log, 1, 0, bytes(lows), records=count)
    return Call(0x546c80, [], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
