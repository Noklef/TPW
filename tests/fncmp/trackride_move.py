# A track ride's car moved (src/game/world/trackrides: 0x547f50 TrakCar_Move) against the original, with its callees run: the car's
# velocity (+0x3c, +0x40) scaled by its ride's rate (+0x24; 7/10 with flag 0x80000, 1000 for a ride of +0x54 2), its ride of one of the
# kinds of each group (the spinning ones, the steered ones, the boats, the rest) with +0x50 2 mostly; the game's frame byte (0x877b5c) on a
# 32nd now and then and its seed (0x877b90) often one whose next roll spawns; the car's instance (animator.py's world, its holder's part
# states 8 entries of parts or none, flag 0x10 or not; its model's root for the seat effect) behind [0x7a4610 + 4 * (+8)], its part
# indices (+0x24, +0x28) and particle system (+0x2c, particles.py's spawn's systems) -1 now and then; its sound (+0x20) 0 or not
# (rssescream.py's manager), the ride's object found among thingenv's park (its id one of ridesound.py's records, some linking an array
# of 11 or more); a link (+0x98) of flag 0x100 or not with a gate (0xdac at +0x10) whose instance (a second animator) is behind
# [0x7a4610 + 4 * (+0xc)] or none, the car near its line or not, asleep (0x200) or not: the car put to sleep and woken through
# Holder_Sleep / Holder_Wake (a renderer with no nodes). Now and then a speed at the boats' thresholds exactly.
# usage: trackride_move.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=SpinA,SteerB,BoatC,Other)
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
import animator  # noqa: E402
import holder  # noqa: E402
import particles  # noqa: E402
import thingenv  # noqa: E402
from fncmp import LOG, RAM, Bench, Call, blob, log_region, put, set_return, vtable  # noqa: E402

B = Bench([(0x547f50, 0x548e40), (0x548e80, 0x54901e), (0x54bbe0, 0x54bc9c), (0x516130, 0x516160), (0x516160, 0x5161b0), (0x50b350, 0x50b360),
           (0x4cd300, 0x4cd331), (0x51bc40, 0x51bc70), (0x51c270, 0x51c2bc), (0x6b5b60, 0x6b5b75), (0x6b5b80, 0x6b5b9a), (0x55a3e0, 0x55a470),
           (0x55a070, 0x55a0b0), (0x46f150, 0x475010), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172), (0x4031e0, 0x403202),
           (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0), (0x429df0, 0x429e17)] + particles.RANGES + holder.RANGES, hooks=holder.HOOKS, sines=256)
MANAGER, VT, ENGINE, EVT = RAM + 0x100, RAM + 0x200, RAM + 0x800, RAM + 0x900
CARANIM, GATEANIM = RAM + 0x10000, RAM + 0x20000
RENDERER = RAM + 0x30000
CAR, RIDES, LINK, GATE, PS, ENTRIES, PARTS, ROOT = (RAM + 0x40000 + 0x400 * k for k in range(8))
RNODES, RARRAYS = RAM + 0x48000, RAM + 0x48800
PNODES, PSINES = RAM + 0x58000, RAM + 0x59000
THINGS = RAM + 0x50000
GLOBALS = 0x877b58
GROUPS = {'SpinA': [-14, -11, -6, -3, -1], 'SteerB': [-13, -12, -8, -5], 'BoatC': [-10, -9, -7, -4], 'Other': [-2, 0, 1, -15, 5]}
NAMES = ('ANIM', 'LAYERS', 'HOLDER', 'MODEL', 'NODES', 'REST', 'RNODES', 'DATA', 'TFLAGS', 'OUT')


def instance(r, base):
    """animator.py's world at base (its holder awake or not, +0x1c no part): (regions, the animator, its holder, its model)."""
    for k, name in enumerate(NAMES):
        setattr(animator, name, base + 0x1000 * k)
    regions, _n, _blocks = animator.world(r)
    put(regions[animator.HOLDER], 0x1c, 'I', 0)
    return regions, animator.ANIM, animator.HOLDER, animator.MODEL


def sound(r, regions):
    log = log_region()
    g = bytearray(r.bytes(0x20))   # 0x802bc0..
    put(g, 8, 'I', r.choice([0, 1, 1, 1, 1]))
    put(g, 0xc, 'I', MANAGER)
    put(g, 0x14, 'I', r.choice([0, 1, 1, 1, 1]))
    blob(log, 2, 1, 0x7f)
    blob(log, 5, 1, 0x7)
    for k in (3, 5):
        if r.random() < 0.6:
            set_return(log, k, r.choice([0, r.getrandbits(32)]))
    regions.update({LOG: log, 0x802bc0: g, MANAGER: struct.pack('<II', VT, ENGINE) + r.bytes(8), VT: vtable({2: 2}, 4),
                    ENGINE: struct.pack('<I', EVT) + r.bytes(0x30), EVT: vtable({3: 3, 5: 2}, 8)})


def ride_records(r, regions):
    """ridesound.py's records (ids 1-8), their arrays 16 long, the count +0x8c 11 or more now and then (element 10)."""
    count = r.choice([0, 1, 2, 3, 4, 4, 4])
    ids = r.sample(range(1, 9), count)
    nodes, arrays = bytearray(r.bytes(0x100 * 4)), bytearray(r.bytes(0x40 * 4))
    for i, rid in enumerate(ids):
        o = 0x100 * i
        put(nodes, o, 'I', RNODES + 0x100 * (i + 1) if i + 1 < count else 0)
        put(nodes, o + 8, 'I', rid)
        put(nodes, o + 0x14, 'I', r.choice(ids + ids + [99]))
        put(nodes, o + 0x1c, 'I', RARRAYS + 0x40 * i)
        put(nodes, o + 0x8c, 'i', r.choice([11, 12, 16, 16, 10, 6, 0, -1]))
        put(arrays, 0x40 * i + 40, 'I', r.choice([0, 1, 0x1d, 0x123, r.getrandbits(32)]))
    regions.update({RNODES: nodes, RARRAYS: arrays, 0x8791a0: struct.pack('<I', r.choice([0, 1, 1, 1, 1])),
                    0x8791b0: struct.pack('<I', RNODES if count else 0)})
    return ids


def parts(r, regions):
    """The part states (+4 the entries): 8 entries, flag 0x10 or not, a part (direction +0x20, position +0x30) or none."""
    entries = bytearray(r.bytes(20 * 8))
    blob_ = bytearray(r.bytes(0x40 * 8))
    for k in range(8):
        entries[20 * k] = r.getrandbits(8) & ~0x10 | r.choice([0, 0x10])
        put(entries, 20 * k + 4, 'I', PARTS + 0x40 * k if r.random() < 0.85 else 0)
        scale = r.choice([1, 1, 10, 300])
        r.floats(blob_, [0x40 * k + o for o in (0x20, 0x24, 0x28)], 1)
        r.floats(blob_, [0x40 * k + o for o in (0x30, 0x34, 0x38)], scale)
        if r.random() < 0.05:
            put(blob_, 0x40 * k + 0x20, 'III', 0, 0, 0)
    regions.update({PS: put(bytearray(r.bytes(0x10)), 4, 'I', ENTRIES), ENTRIES: entries, PARTS: blob_})


def spawn_world(r, regions):
    """particles.py's spawn world (SpawnVel's scales), its nodes and sines moved; a handle to one of its six systems, stale or -1."""
    c = particles.spawn(r)
    p = dict(c.regions)
    p[PSINES], p[PNODES] = p.pop(particles.SINES), p.pop(particles.NODES)
    p[0x80ced0] = put(put(bytearray(p[0x80ced0]), 0x1c, 'I', PSINES), 0x28, 'I', PNODES)
    defs = bytearray(p[0x804370])
    for k in range(0x69):
        put(defs, 0x140 * k + 0xb0, 'i', r.choice([0, 1024, 512, 2048, -1024, r.randrange(-0x10000, 0x10000)]))
    p[0x804370] = defs
    systems = bytearray(p[0x80cefc])
    slot, serial = r.randrange(6), r.getrandbits(16)
    put(systems, 0x140 * slot + 0xa, 'H', serial)
    p[0x80cefc] = systems
    regions.update(p)
    return r.choice([serial << 16 | slot] * 3 + [(serial ^ 1) << 16 | slot, 0xffffffff, 0xffff])


def seed(r, want):
    s = r.getrandbits(32)
    if r.random() < 0.6:
        while ((s * 0x343fd + 0x269ec3) & 0xffffffff) >> 16 & 7 not in want:
            s = r.getrandbits(32)
    return s


def case(r):
    op = r.deal(OPS)
    regions = {}
    cregs, canim, cholder, cmodel = instance(r, CARANIM)
    gregs, ganim, _gholder, _gmodel = instance(r, GATEANIM)
    regions.update(gregs)
    regions.update(cregs)   # (the car's clocks)
    put(regions[cholder], 0x28, 'I', PS)
    put(regions[cmodel], 0x78, 'I', ROOT)
    regions[ROOT] = r.floats(bytearray(r.bytes(0x50)), [0x40, 0x44, 0x48], r.choice([1, 10, 300]))
    parts(r, regions)
    sound(r, regions)
    ids = ride_records(r, regions)
    system = spawn_world(r, regions)
    regions[0x7a4610] = struct.pack('<4I', canim, canim, ganim, canim)
    renderer = put(bytearray(r.bytes(0x100)), 4, 'I', 0)
    renderer[0x14:0x30] = bytes(0x1c)
    put(renderer, 0xc, 'I', 5)
    regions.update({0x87a6d0: put(bytearray(r.bytes(0x10)), 0, 'I', RENDERER), RENDERER: renderer, 0x8bd500: r.bytes(0x20)})
    # the ride, its globals
    kind = r.choice(GROUPS[op]) & 0xffffffff
    rides = bytearray(r.bytes(4 * 0xd0))
    index = r.randrange(4)
    ride = RIDES + 0xd0 * index
    put(rides, 0xd0 * index, 'I', kind)
    put(rides, 0xd0 * index + 0x24, 'i', r.choice([1000, 1000, 1024, 700, 0, -1000, r.randrange(-3000, 3000), r.getrandbits(32) - (1 << 31)]))
    put(rides, 0xd0 * index + 0x50, 'I', r.choice([2, 2, 2, 1, 0]))
    put(rides, 0xd0 * index + 0x54, 'I', r.choice([1, 1, 2, 0, 3]))
    globals_ = bytearray(r.bytes(0x40))
    put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.95 else r.getrandbits(32))
    globals_[4] = r.choice([0, 0x20, 0x40, r.getrandbits(8)])
    put(globals_, 8, 'I', RIDES)
    put(globals_, 0x38, 'I', seed(r, (2,) if op == 'SpinA' else (0, 1, 2)))
    name = (index | kind << 8) & 0xffffffff
    # the park's things: the ride's object (some), ids from the records
    park, handles, _things = thingenv.park(r, count=r.choice([0, 1, 2, 3, 4, 6]), kinds=(3, 3, 3, 1, 2, 0), defs=False, pool=THINGS)
    regions.update(park)
    pool = regions[THINGS]
    for i in range(len(handles)):
        put(pool, thingenv.THING * i + 0x28, 'I', name if r.random() < 0.5 else r.choice([name ^ 1, name ^ 0x100, 0, r.getrandbits(32)]))
        put(pool, thingenv.THING * i + 0x24, 'I', r.choice(ids + ids + [99, 0] if ids else [1, 2, 0]))
    # the car
    car = bytearray(r.bytes(0xac))
    flags = r.getrandbits(32) & ~(0x80000 | 0x4000 | 0x400 | 0x200 | 0x800000 | 0x2002000)
    for bit in (0x80000, 0x400, 0x200, 0x800000, 0x2000, 0x2000000):
        if r.random() < 0.3:
            flags |= bit
    if r.random() < 0.7:
        flags |= 0x4000
    put(car, 0, 'I', flags)
    put(car, 8, 'II', r.choice([0, 1, 1, 3]), r.choice([0, 0, 1, 2]))
    put(car, 0x10, 'I', r.choice([5, 5, 0xc, 0xc, 0, 2]))
    put(car, 0x14, 'I', r.choice([0, 0, 0, 1, 2, 4]))
    put(car, 0x20, 'I', r.choice([0, r.getrandbits(32), r.getrandbits(32)]))
    put(car, 0x24, 'iiI', r.choice([-1, 0, 7, r.randrange(8), r.randrange(8)]), r.choice([-1, 0, 7, r.randrange(8), r.randrange(8)]), system)
    put(car, 0x30, 'I', 0)
    coord = lambda: r.choice([0, 300, -300, r.randrange(-100000, 100000), r.getrandbits(32) - (1 << 31)]) & 0xffffffff  # noqa: E731
    put(car, 0x34, 'II', coord(), coord())
    v = lambda: r.choice([0, 1, -1, r.randrange(-300, 300), r.randrange(-300, 300), r.randrange(-3000, 3000), r.getrandbits(32) - (1 << 31)])  # noqa: E731
    put(car, 0x3c, 'ii', v(), v())
    if r.random() < 0.15:   # a speed at the boat thresholds exactly: rate 1024, one axis
        edge = r.choice([0x28, 0x29, 0x50, 0x51, 0xe6, 0xe7])
        put(car, 0x3c, 'ii', *r.choice([(edge, 0), (0, edge), (-edge, 0), (0, -edge)]))
        put(car, 0, 'I', struct.unpack_from('<I', car, 0)[0] & ~0x80000)
        put(rides, 0xd0 * index + 0x24, 'i', 1024)
        put(rides, 0xd0 * index + 0x54, 'I', 1)
    put(car, 0x50, 'i', r.choice([0, 0x100, 0x1ff, r.randrange(0x200), r.getrandbits(32) - (1 << 31)]))
    put(car, 0x54, 'i', r.choice([0, 0x100, 0x1ff, r.randrange(0x200), r.randrange(0x200), r.getrandbits(32) - (1 << 31)]))
    put(car, 0x58, 'i', r.choice([0, 0x30, -0x30, 0x31, -0x31, 0x7f, -0x80, 0x880, -0x881, r.randrange(-0x1000, 0x1000), r.getrandbits(32) - (1 << 31)]))
    put(car, 0x5c, 'i', r.randrange(-0x20, 0x20))
    put(car, 0x9c, 'I', ride)
    # the link and its gate
    has_link = r.random() < 0.6
    put(car, 0x98, 'I', LINK if has_link else 0)
    link = bytearray(r.bytes(0x40))
    put(link, 0, 'I', r.getrandbits(32) & ~0x100 | (0x100 if r.random() < 0.8 else 0))
    cx, cy = struct.unpack_from('<II', car, 0x34)
    near = lambda c: (c + r.choice([0, 0x4af, -0x4af, 0x4b0, -0x4b0, 0x4b1, -0x4b1, r.randrange(-0x800, 0x800), r.randrange(-0x800, 0x800), r.getrandbits(32)])) & 0xffffffff  # noqa: E731
    put(link, 4, 'II', near(cx), near(cy))
    put(link, 0x18, 'I', GATE if r.random() < 0.9 else 0)
    gate = bytearray(r.bytes(0x40))
    put(gate, 0, 'I', r.getrandbits(32) & ~2 | r.choice([0, 2]))
    put(gate, 4, 'I', r.choice([0xc, 0xc, 0xd, 0]))
    put(gate, 8, 'I', r.choice([0, 0, 1]))
    put(gate, 0xc, 'I', r.choice([2, 2, 1, 0]))
    put(gate, 0x10, 'I', 0xdac if r.random() < 0.9 else r.getrandbits(32))
    regions.update({CAR: car, RIDES: rides, LINK: link, GATE: gate, GLOBALS: globals_, RAM: r.bytes(4),
                    0x7cf6ec: RAM.to_bytes(4, 'little')})
    return Call(0x547f50, [CAR], regions, ret='v')


OPS = [k for k in os.environ.get('OPS', ','.join(GROUPS)).split(',') if k]


if __name__ == '__main__':
    B.main(case)
