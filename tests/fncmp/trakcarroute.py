# A track ride's car choosing what to do next (src/game/world/trakcarroute: TrakCar_ChooseRoute 0x54a040) against the original. Its calls that reach
# past the module are cdecl recorders (the driver's TrakCarRouteEnv): Model_FindPartByKey4 0x44b220 (1), Ride_Create 0x463060 (2), Sound_PlayRideElement
# 0x51eeb0 (3), Sound_SetPosition 0x51c270 (4) and IntArea_Overlaps 0x5491e0 (5). World_Rand, the thing list's functions, Thing_ByHandle,
# TrakRide_FindThing's search and ObjectDefs_Find run as they are.
# The world: the car pool (some used, the car among them or not), a ride of every kind with a route (flags with 2: a node without it spins forever), the
# array of other nodes, the sine table, thingenv's park with objects of the ride, the object definitions.
# usage: trakcarroute.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402
import thingenv  # noqa: E402

HOOKS = {0x44b220: _recorder(1, 3, cdecl=True), 0x463060: _recorder(2, 10, cdecl=True), 0x51eeb0: _recorder(3, 7, cdecl=True),
         0x51c270: _recorder(4, 4, cdecl=True), 0x5491e0: _recorder(5, 2, cdecl=True)}
B = Bench([(0x54a040, 0x54aa41), (0x516330, 0x516370), (0x516130, 0x5161b0), (0x50b350, 0x50b360), (0x4cd300, 0x4cd331), (0x5da3c0, 0x5da3c1),
           (0x412e90, 0x412eea)], hooks=HOOKS)
HIGH = RAM + 0x40000
POOL, RIDES, ROUTE, NODES, AREA, STATE, DEF = HIGH, HIGH + 0x2000, HIGH + 0x3000, HIGH + 0x3400, HIGH + 0x3800, HIGH + 0x3900, HIGH + 0x4000
N_POOL, N_RIDES, N_ROUTE, N_NODES = 12, 3, 8, 6
SINES = b''.join(struct.pack('<i', round(256 * math.sin(2 * math.pi * i / 0x200))) for i in range(0x200))
WANDER, FOLLOW, NEWFOLLOW, START = [-14, -11, -6, -3, -1], [-7, -4], [-10, -9], [-13, -12, -8, -5]


def route(r):
    nodes = bytearray(r.bytes(0x1c * N_ROUTE))
    n = r.randrange(1, N_ROUTE + 1)
    for i in range(n):
        p = 0x1c * i
        put(nodes, p, 'I', r.getrandbits(32) | 2)
        put(nodes, p + 4, 'I', r.choice([i, r.randrange(N_NODES), r.getrandbits(32)]))
        put(nodes, p + 0x14, 'I', ROUTE + 0x1c * (i + 1) if i + 1 < n else 0)
    return nodes, n


def array(r):
    nodes = bytearray(r.bytes(0x1c * N_NODES))
    for i in range(N_NODES):
        p = 0x1c * i
        put(nodes, p + 4, 'I', r.choice([i, r.getrandbits(32)]))
        put(nodes, p + 0x14, 'I', r.choice([0, ROUTE, ROUTE + 0x1c, NODES + 0x1c * r.randrange(N_NODES)]))
    return nodes


def near(r, pool, n_pool, self_, rides, at):
    """A new wandering car at the ride's very place (its area's size 1) and one other used car just inside, on or just outside its reach, at the
    distances where the halving starts or the farthest (16 halvings)."""
    p = 0xac * self_
    far = r.random() < 0.3
    x, y = (-0x3fffff00, -0x3fffff00) if far else (r.randrange(-1000, 1000), r.randrange(-1000, 1000))
    put(rides, at + 0x74, 'ii', x, y)
    reach = r.randrange(0, 64)
    put(pool, p + 0x64, 'i', reach)
    put(pool, p, 'I', struct.unpack_from('<I', pool, p)[0] | 0x100000)
    dx, dy = r.choice([(0x7ffffe00, r.randrange(0x7ffffe00)), (r.randrange(0x7ffffe00), 0x7ffffe00)]) if far else \
        r.choice([(0x8000, 0x7fff), (0x8001, 0), (0x8000, 0), (0, 0x7fff), (0, 0x8000), (0x7fff, 0x8000), (r.randrange(0x10000), r.randrange(0x10000))])
    other = r.choice([i for i in range(n_pool) if i != self_])
    for i in range(n_pool):
        if i != self_:
            put(pool, 0xac * i, 'I', struct.unpack_from('<I', pool, 0xac * i)[0] & ~1 | (i == other))
    put(pool, 0xac * other + 0x34, 'ii', x + dx, y + dy)
    halved = 0
    while dx > 0x8000 or dy > 0x7fff:
        dx, dy, halved = dx >> 1, dy >> 1, halved + 1
    distance = int(math.sqrt(dx * dx + dy * dy)) << halved
    put(pool, 0xac * other + 0x64, 'I', (distance - reach + r.choice([-1, 0, 0, 1])) & 0xffffffff)


def last_partners(pool, self_, ride):
    """The partner's index past 42 (the others not partners): tells the size 0xac apart, and the car free to look for one."""
    for i in range(46):
        flags = struct.unpack_from('<I', pool, 0xac * i)[0]
        if i != self_:
            put(pool, 0xac * i, 'I', flags & ~0x20 | 1 if i >= 42 else flags | 0x20)
            put(pool, 0xac * i + 0x9c, 'I', ride)
    put(pool, 0xac * self_, 'I', struct.unpack_from('<I', pool, 0xac * self_)[0] & ~0x40004)


def case(r):
    log = log_region()
    close = r.random() < 0.2
    kind = r.choice(WANDER) if close else r.deal(WANDER + WANDER + FOLLOW + FOLLOW + NEWFOLLOW + START + [-2, -2, 0, 1, -15, 7])
    nodes, n = route(r)
    rides = bytearray(r.bytes(0xd0 * N_RIDES))
    index = r.randrange(N_RIDES)
    ride, at = RIDES + 0xd0 * index, 0xd0 * index
    put(rides, at, 'I', kind & 0xffffffff)
    put(rides, at + 0x10, 'I', r.choice([1, 2, 3, 0x105]))
    put(rides, at + 0x20, 'i', r.choice([0, 9, 100, -100, r.randrange(-100000, 100000)]))
    put(rides, at + 0x50, 'I', r.deal([1, 2, 2, 1, 2, 0, 3]))
    put(rides, at + 0x5c, 'i', r.choice([0, 1, 2, 3, n, -1] if r.random() < 0.5 else [1, 2, 3, n]))
    if struct.unpack_from('<i', rides, at + 0x5c)[0] > n:
        put(rides, at + 0x5c, 'i', n)
    put(rides, at + 0x74, 'ii', r.randrange(-100000, 100000), r.randrange(-100000, 100000))
    put(rides, at + 0xa4, 'III', ROUTE + 0x1c * r.randrange(n), ROUTE, r.randrange(1, 12))
    put(rides, at + 0xc0, 'I', AREA)
    area = bytearray(0x40)
    put(area, 0xc, 'I', 1 if close else r.choice([1, 8, 100, 4096, 65536, 0x100000]))
    n_pool = r.choice([N_POOL, N_POOL, 46])   # (the chosen car's index past 42 tells the size 0xac apart)
    pool = bytearray(0xac * n_pool)
    self_ = r.randrange(n_pool)
    used = r.random()
    for i in range(n_pool):
        p = 0xac * i
        put(pool, p, 'I', (r.getrandbits(32) & ~1) | (1 if r.random() < used or i == self_ else 0))
        if r.random() < 0.3:
            put(pool, p, 'I', struct.unpack_from('<I', pool, p)[0] & ~0x20)
        put(pool, p + 0x34, 'ii', r.randrange(-200000, 200000), r.randrange(-200000, 200000))
        put(pool, p + 0x64, 'i', r.choice([0, 1, 64, 4096, 0x10000, 0x20000000, r.randrange(0, 0x10000)]))
        put(pool, p + 0x9c, 'I', r.choice([ride, ride, RIDES + 0xd0 * ((index + 1) % N_RIDES)]))
        put(pool, p + 0x98, 'I', r.getrandbits(32))
    car, p = POOL + 0xac * self_, 0xac * self_
    fields = r.deal([0, 0x100000, 0x100000, 0x100000, 0x404000, 0x100000 | 0x404000, 8, 0x100000 | 8, 0x100000 | 0x2000, 0x40000, 4, 0x20 | 0x404000])
    put(pool, p, 'I', (r.getrandbits(32) & ~0x54602d) | 1 | fields)
    if r.random() < 0.3:
        put(pool, p, 'I', r.getrandbits(32) | 1)
    put(pool, p + 8, 'I', r.choice([0, 1, 2]))
    put(pool, p + 0x20, 'I', r.choice([0, 0, 0, 1]))
    put(pool, p + 0x7c, 'i', r.choice([0, 1, N_NODES - 1] if r.random() < 0.7 else [-1, N_NODES, 0x7fffffff]))
    put(pool, p + 0x9c, 'I', ride)
    if struct.unpack_from('<i', pool, p + 0x7c)[0] not in range(N_NODES) and kind == -2 or fields & 8 and kind in WANDER:
        put(pool, p + 0x7c, 'i', r.randrange(N_NODES))
    if close:
        near(r, pool, n_pool, self_, rides, at)
    elif n_pool == 46 and r.random() < 0.5:
        last_partners(pool, self_, ride)
    pool_count = sum(struct.unpack_from('<I', pool, 0xac * i)[0] & 1 for i in range(n_pool))
    globals_ = bytearray(r.bytes(0x28))
    put(globals_, 0, 'I', 0x4a454647 if r.random() < 0.9 else r.getrandbits(32))
    put(globals_, 8, 'I', RIDES)
    put(globals_, 0x10, 'II', POOL, r.choice([pool_count, pool_count, pool_count - 1, 0, 1]))
    put(globals_, 0x20, 'Ii', NODES, r.choice([N_NODES, 1, 2]))
    park, handles, _things = thingenv.park(r, count=r.choice([0, 1, 2, 3, 4, 6]), kinds=(3, 3, 3, 1, 2, 0), defs=False)
    regions = dict(park)
    name = (index | (kind & 0xffffffff) << 8) & 0xffffffff
    things = regions[thingenv.POOL]
    for i in range(len(handles)):
        put(things, thingenv.THING * i + 0x28, 'I', name if r.random() < 0.5 else r.choice([name ^ 1, name ^ 0x100, 0, r.getrandbits(32)]))
        put(things, thingenv.THING * i + 0x24, 'I', r.getrandbits(32))
    defs = bytearray(0x738)   # (0x7890a0: the object definitions, one cached or found in the list)
    put(defs, 0, 'I', DEF)
    put(defs, 0x528, 'i', 1)
    wanted = struct.unpack_from('<H', rides, at + 0x10)[0]
    put(defs, 0x730, 'H', wanted if r.random() < 0.5 else 0xffff)
    put(defs, 0x734, 'I', DEF)
    definition = bytearray(0x500)
    put(definition, 0, 'H', wanted)
    put(definition, 0x4bc, 'I', r.getrandbits(32))
    table = bytearray(0xc)
    for i in range(3):
        put(table, 4 * i, 'I', r.getrandbits(32))
    set_return(log, 1, r.getrandbits(32))
    set_return(log, 2, r.getrandbits(32))
    set_return(log, 3, r.choice([0, 1, r.getrandbits(32)]))
    set_return(log, 4, r.getrandbits(32))
    set_return(log, 5, r.choice([0, 1, 1]))
    regions.update({LOG: log, POOL: pool, RIDES: rides, ROUTE: nodes, NODES: array(r), AREA: area, DEF: definition, 0x7890a0: defs,
                    STATE: struct.pack('<I', r.getrandbits(32)), 0x7cf83c: struct.pack('<I', (STATE - 0x1da708) & 0xffffffff),
                    0x877358: SINES, 0x877b58: globals_, 0x7a4610: table, 0x803a3c: r.bytes(4)})
    return Call(0x54a040, [ride, car], regions)


if __name__ == '__main__':
    B.main(case)
