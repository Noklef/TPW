# A car taken off its track ride (src/game/world/trackrides: 0x54ae50 TrakCar_Remove) against the original, with its callees run: the car
# (flags 1 / 0x400000 / 0x2000 / 0x2000000 either way, its instance index 0-3, a second ride +0xc, its sound +0x20 none now and then, a
# particle system +0x2c -1 or a live, stale or any handle on particles.py's Spawn state, 0-3 seat nodes without parts on +0x30, its
# position +0x34 / +0x38 any); its ride (+0x9c) one of four records (the kinds -13..-4 mostly, others now and then; the state +0x50 4 or
# not; the count +0x5c 1 mostly); the instance's holder's model's root position (trackride_seats.py's chain); Ride_Destroy (0x463c80) a
# cdecl recorder (31). The sound: for the kinds that play, ridesound.py's records and manager (moved), its manager's engine (+4) slot 5
# Sound_SetPosition's recorder (5, the position hashed), thingenv's park (moved) with objects of the ride's name (the module loaded or
# not) or others; for the rest soundset.py's sender world for Sound_Stop (0x51c300).
# usage: trackcar_remove.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import HEAP, LOG, RAM, Bench, Call, _recorder, blob, heap, put, vtable  # noqa: E402
_ops = os.environ.pop('OPS', None)   # (the imported benches read OPS as their own)
import particles  # noqa: E402
import ridesound as rs  # noqa: E402
import soundset  # noqa: E402
import thingenv  # noqa: E402
if _ops is not None:
    os.environ['OPS'] = _ops
rs.OPS = ['Play']
particles.NODES, particles.SINES = RAM + 0x60000, RAM + 0x61000   # (clear of the sound object's page)
rs.NODES, rs.ARRAYS, rs.MANAGER, rs.VT = RAM + 0x50000, RAM + 0x51000, RAM + 0x52000, RAM + 0x52100
ENGINE, EVT = RAM + 0x52200, RAM + 0x52300
CAR, RIDES, CNODES, INSTANCE, HOLDER, MODEL, ROOT = (RAM + o for o in (0x30000, 0x31000, 0x32000, 0x33000, 0x33100, 0x33200, 0x33300))
POOL = RAM + 0x40000
PLAY = (-10, -9, -7, -4)


def merged(ranges):   # (overlapping ranges joined: a listing line must not come twice)
    out = []
    for lo, hi in sorted(ranges):
        if out and lo <= out[-1][1]:
            out[-1] = (out[-1][0], max(out[-1][1], hi))
        else:
            out.append((lo, hi))
    return out


B = Bench(merged(particles.RANGES + soundset.RANGES + [
    (0x54ae50, 0x54b0f8), (0x54ac70, 0x54ad8e), (0x67a830, 0x67a860), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360),
    (0x4cd300, 0x4cd331), (0x51eeb0, 0x51eeec), (0x55a3e0, 0x55a470), (0x55a070, 0x55a0b0), (0x51bfc0, 0x51c048)]),
    hooks={0x463c80: _recorder(31, 1, cdecl=True)})


def case(r):
    regions = dict(particles.spawn(r).regions)
    defs = regions[0x804370]
    for k in range(0x69):   # (no voice marks: as trackride_seats.py)
        if defs[0x140 * k + 0xba]:
            put(defs, 0x140 * k + 0xb8, 'H', 0)
    index = r.randrange(4)
    rides = bytearray(r.bytes(4 * 0xd0))
    kind = r.choice(list(range(-13, -3)) * 3 + [-14, -3, 0, 1, r.getrandbits(32) - (1 << 31)])
    ride = RIDES + 0xd0 * index
    put(rides, 0xd0 * index, 'i', kind)
    put(rides, 0xd0 * index + 0x50, 'I', r.choice([4, 4, 0, 1, 2, 3, r.getrandbits(32)]))
    put(rides, 0xd0 * index + 0x5c, 'i', r.choice([1, 1, 1, 2, 0, 5, -1]))
    put(rides, 0xd0 * index + 0x60, 'i', r.randrange(-2, 8))
    module = bytearray(r.bytes(0x40))
    if r.random() < 0.8:
        put(module, 0, 'I', 0x4a454647)
    put(module, 8, 'I', RIDES)
    name = (index | (kind & 0xffffffff) << 8) & 0xffffffff if struct.unpack_from('<I', module)[0] == 0x4a454647 else 0
    car = bytearray(r.bytes(0xac))
    put(car, 0, 'I', r.getrandbits(32) & ~0x2402001 | r.choice([0, 0x400000]) | r.choice([0, 0, 0x2000, 0x2000000, 0x2002000]) | r.choice([0, 1]))
    slot = r.randrange(4)
    put(car, 8, 'I', slot)
    n = r.choice([0, 0, 1, 2, 3])
    nodes = bytearray(r.bytes(0x20 * 3))
    for k in range(n):
        put(nodes, 0x20 * k + 8, 'i', -1)
        put(nodes, 0x20 * k + 0x10, 'I', CNODES + 0x20 * (k + 1) if k + 1 < n else 0)
    put(car, 0x30, 'I', CNODES if n else 0)
    put(car, 0x20, 'I', r.choice([0, 0x1d, 0x123, r.getrandbits(32)]))
    systems = regions[0x80cefc]
    live = r.randrange(6)
    put(car, 0x2c, 'I', r.choice([0xffffffff] * 3 + [struct.unpack_from('<H', systems, 0x140 * live + 0xa)[0] << 16 | live] * 3
                                 + [0, live, r.getrandbits(32)]))
    coord = lambda: r.choice([0, 1, -1, 300, -300, r.randrange(-100000, 100000), r.getrandbits(32)]) & 0xffffffff  # noqa: E731
    put(car, 0x34, 'II', coord(), coord())
    put(car, 0x9c, 'I', ride)
    root = bytearray(r.bytes(0x50))
    put(root, 0x40, '3f', *(r.choice([r.uniform(-300, 300), r.uniform(-300, 300), r.uniform(-3e6, 3e6), 0.0]) for _ in range(3)))
    regions.update({CAR: car, RIDES: rides, CNODES: nodes, 0x877b58: module, ROOT: root, HEAP: heap(),
                    0x7a4610: b''.join(struct.pack('<I', INSTANCE if k == slot else r.getrandbits(32)) for k in range(4)),
                    INSTANCE: r.bytes(8) + struct.pack('<I', HOLDER) + r.bytes(4), HOLDER: r.bytes(4) + struct.pack('<I', MODEL) + r.bytes(8),
                    MODEL: r.bytes(0x78) + struct.pack('<I', ROOT) + r.bytes(4)})
    if kind in PLAY:
        regions.update(rs.case(r).regions)
        regions[rs.MANAGER] = struct.pack('<II', rs.VT, ENGINE) + r.bytes(8)
        regions.update({ENGINE: struct.pack('<I', EVT) + r.bytes(0x30), EVT: vtable({5: 2})})
        blob(regions[LOG], 5, 1, 7)
        park, handles, _things = thingenv.park(r, count=r.choice([0, 1, 2, 3, 4, 6]), kinds=(3, 3, 3, 1, 2, 0), defs=False, pool=POOL)
        regions.update(park)
        pool = regions[POOL]
        for i in range(len(handles)):
            put(pool, thingenv.THING * i + 0x28, 'I', name if r.random() < 0.5 else r.choice([name ^ 1, name ^ 0x100, 0, r.getrandbits(32)]))
            put(pool, thingenv.THING * i + 0x24, 'I', r.choice([1, 2, 3, 4, 5, 6, 7, 8, r.getrandbits(32)]))
        regions[0x803a3c] = r.bytes(4)
    else:
        stop = soundset.sender_case(r, 0x51c300)
        regions.update(stop.regions)
        if r.random() < 0.7:   # (the id one of the voices' mostly, as the sender case draws it)
            put(car, 0x20, 'I', stop.args[0] or 1)
    return Call(0x54ae50, [CAR], regions, ret='a')


if __name__ == '__main__':
    B.main(case)
