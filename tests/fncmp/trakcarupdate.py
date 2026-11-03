# A track ride's car stepping (src/game/world/trakcarupdate: TrakCar_Update 0x5474b0) against the original. Its calls are cdecl recorders (the driver's TrakCarUpdateEnv):
# Collider_SteerTo 0x547c60 (1 of 3, about the ride's reach), TrakCar_ChooseRoute 0x54a040 (2 of 2), CollArea_FindNearest 0x549070 (3 of 4), TrakRide_Unload 0x54ac70 (4 of 2),
# TrakCar_Remove 0x54ae50 (5 of 1), Sound_Stop 0x51c300 (6 of 1), Layer_ClearQueue 0x473270 (7 of 2), Animator_Play 0x4732a0 (8 of 6); IntArea_Contains and World_Rand run as they
# are. The fixture: the car in a pool of 3, its ride of each kind and state, nodes on a chain with flags and areas, the module's areas, and the flags picking each path.
# usage: trakcarupdate.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, log_region, put, set_return  # noqa: E402

HOOKS = {0x547c60: _recorder(1, 3, cdecl=True), 0x54a040: _recorder(2, 2, cdecl=True), 0x549070: _recorder(3, 4, cdecl=True), 0x54ac70: _recorder(4, 2, cdecl=True),
         0x54ae50: _recorder(5, 1, cdecl=True), 0x51c300: _recorder(6, 1, cdecl=True), 0x473270: _recorder(7, 2, cdecl=True), 0x4732a0: _recorder(8, 6, cdecl=True)}
B = Bench([(0x5474b0, 0x547c04), (0x5493f0, 0x5494c6), (0x516330, 0x516370), (0x67a830, 0x67a860), (0x5da3c0, 0x5da3c1)], hooks=HOOKS)
POOL, RIDE, NODES, AREAS, NEAR, STATE = RAM, RAM + 0x300, RAM + 0x400, RAM + 0x600, RAM + 0x800, RAM + 0x900
KINDS = [-14, -11, -6, -3, -1, -10, -9, -7, -4, -2, -5, -13]
# Flags that pick each path (bits past them random).
PATHS = [4, 4, 4, 0x20, 0x20, 8, 8, 8, 8, 0x48, 0x48, 0x10, 0x10, 0x40, 8 | 0x8000, 0]
AREA_XY = 30000


def area(r, x, y):
    flags = r.choice([1, 1, 0x41, 0]) | r.choice([0, 6, 0x18, 0x1e, 0x3e, 0x3e])
    return struct.pack('<5i', flags, x + r.randrange(-500, 500), y + r.randrange(-500, 500), r.choice([100, 1000, 4000]), r.choice([100, 1000, 4000]))


def vector(r):
    return r.choice([0, 0, 1, -1, r.randrange(-300, 300), r.randrange(-0x20000, 0x20000), r.randrange(-0x20000, 0x20000), -1 << 31]), r.randrange(-0x20000, 0x20000)


def case(r):
    log = log_region()
    x, y = r.randrange(-AREA_XY, AREA_XY), r.randrange(-AREA_XY, AREA_XY)
    pool = bytearray(r.bytes(0xac * 3))
    spot = r.randrange(3)
    car = POOL + 0xac * spot
    reach = r.choice([0x100, 0x400, 0x1000, r.randrange(1, 0x8000)])
    set_return(log, 1, r.choice([reach, reach - 1, reach + 1, reach // 4, reach // 4 + 1, 0, r.randrange(0, 2 * reach), r.randrange(reach, 0x10000)]))
    set_return(log, 4, r.choice([0, 0, 1, r.getrandbits(32)]))
    set_return(log, 3, NEAR)
    set_return(log, 2, r.getrandbits(32))
    node_count = r.randrange(1, 7)
    regions_areas = bytearray()
    for i in range(8):
        regions_areas += area(r, x, y) + bytes(0xc)
    in_use = sum(struct.unpack_from('<I', regions_areas, 0x20 * i)[0] & 1 for i in range(8))
    nodes = bytearray(r.bytes(0x1c * node_count))
    for i in range(node_count):
        k = r.random()
        fl = 1 if k < 0.85 else 0
        fl |= r.choice([0, 0, 4, 4, 2])
        if i == 0:
            fl &= ~2
        put(nodes, 0x1c * i, 'I', fl)
        put(nodes, 0x1c * i + 4, 'I', r.randrange(0, node_count))
        put(nodes, 0x1c * i + 8, '2i', r.randrange(-40000, 40000), r.randrange(-40000, 40000))
        put(nodes, 0x1c * i + 0x14, 'I', NODES + 0x1c * (i + 1) if i + 1 < node_count and r.random() < 0.8 else 0)
        put(nodes, 0x1c * i + 0x18, 'I', r.choice([0, 0, AREAS + 0x20 * r.randrange(8)]))
    ride = bytearray(r.bytes(0xb0))
    put(ride, 0, 'i', r.deal(KINDS))
    put(ride, 0x50, '2I', r.choice([0, 1, 2, 2, 3]), r.choice([0, 1, 2, 3]))
    put(ride, 0x68, 'i', reach)
    put(ride, 0x90, '2i', r.randrange(-40000, 40000), r.randrange(-40000, 40000))
    put(ride, 0xa4, '2I', NODES + 0x1c * r.randrange(node_count), NODES)
    for j in range(3):
        at = 0xac * j
        put(pool, at, 'I', r.choice([1, 1, 1, 1, 0x21, 0x40001, 0, 3]) | (r.getrandbits(32) & ~0x40021))
        ox, oy = x + r.randrange(-30000, 30000), y + r.randrange(-30000, 30000)
        if r.random() < 0.03:
            ox ^= 1 << 30
        vx, vy = vector(r)
        put(pool, at + 0x34, '4i', ox, oy, vx, vy)
        put(pool, at + 0x98, 'I', r.choice([0, AREAS + 0x20 * r.randrange(8), AREAS + 0x20 * r.randrange(8)]))
    me = 0xac * spot
    path = r.deal(PATHS)
    junk = r.getrandbits(32) & ~0x7c | (r.getrandbits(32) & 0x10000000)
    flags = junk & 0xfff00003 | path | r.choice([0, 0x20000, 0x4000, 0x20000 | 0x4000, 0x8000])
    if r.random() < 0.1:
        flags |= r.getrandbits(32) & 0x7c
    put(pool, me, 'I', flags & 0xffffffff)
    put(pool, me + 8, 'I', r.choice([0, 1, 7]))
    put(pool, me + 0x10, 'I', r.choice([0xc, 3, r.getrandbits(32)]))
    put(pool, me + 0x20, 'I', r.choice([0, 0x80000011]))
    put(pool, me + 0x34, '2i', x, y)
    put(pool, me + 0x7c, '2I', r.randrange(node_count), r.randrange(3))
    put(pool, me + 0x88, '2i', r.choice([-1, 0, 1, 1, 2, r.randrange(3, 100)]), r.choice([0, 1, 2, 2, 3, r.randrange(4, 100), -3]))
    put(pool, me + 0x9c, 'I', RIDE)
    near = struct.pack('<4i', r.getrandbits(32) & 0xff, x + r.randrange(-5000, 5000), y + r.randrange(-5000, 5000), r.choice([0, 100, 1000, r.randrange(-500, 5000)]))
    instances = bytes(r.getrandbits(8) | 0x80 if i % 4 == 3 else r.getrandbits(8) for i in range(32))
    areas_ptr, count = (AREAS, r.choice([in_use, in_use, max(in_use - 1, 0)])) if r.random() < 0.9 else (0, in_use)
    node_ptr, node_n = (NODES, node_count) if r.random() < 0.95 else (0, node_count)
    regions = {LOG: log, POOL: pool, RIDE: ride, NODES: nodes, AREAS: regions_areas, NEAR: near, STATE: struct.pack('<I', r.getrandbits(32)),
               0x7cf83c: struct.pack('<I', (STATE - 0x1da708) & 0xffffffff), 0x7a4610: instances, 0x877b68: struct.pack('<I', POOL), 0x877b70: struct.pack('<2I', areas_ptr, count),
               0x877b78: struct.pack('<2I', node_ptr, node_n)}
    return Call(0x5474b0, [car], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
