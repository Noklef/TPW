# A ride object swung by its running-down angle (0x468e50, src/game/coaster/orient) against the original: the game clock 0x785970 as
# animator.py's UpdateClocks has it (paused, frozen or not, on the game's Timer and its imports: thunks_static.py's clock()), the object
# (+0 set or not, +4 a ride index 0-1, +8 a matrix, +0x48 the angle: whole and half steps of the sine table or any float, +0x4c-+0x6c
# floats, +0x70 a time), the ride table 0x7a4610 to the holder chain (+8 -> +4 -> +0x78) and its node's matrix (+0x10).
# usage: rideswing.py [CASES [SEED]] [--wild|--both] [--show N]
import math
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', '..', 'tools'))
sys.path.insert(0, HERE)
from fncmp import Bench, Call, RAM, put  # noqa: E402
from thunks_static import clock as timer  # noqa: E402

B = Bench([(0x468e50, 0x46909c), (0x46f240, 0x46f350), (0x46f420, 0x46f4f0), (0x402d70, 0x402e00), (0x402e60, 0x4030f0), (0x403150, 0x403172),
           (0x4031e0, 0x403202), (0x4033a0, 0x4033f0), (0x5f5f10, 0x5f5fc0)], sines=4096)
OBJ, RIDES, HOLDER, INNER, NODE = (RAM + 0x100 * k for k in range(5))


def case(r):
    regions = {}
    timer(r, regions, True)
    c = bytearray(r.bytes(0xa0))
    put(c, 0x30, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(c, 0x68, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    put(c, 0x38, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    for o in (0x18, 0x58):
        put(c, o, 'd', r.choice([0.0, 1.0, 1000.5, float(r.getrandbits(32)), r.uniform(0, 2 ** 31)]))
    put(c, 0x20, 'd', r.choice([1.0, 1.0, 0.5, 2.0, 0.0, r.uniform(0, 4)]))
    regions[0x785970] = c

    obj = bytearray(r.bytes(0x80))
    put(obj, 0, 'I', r.choice([0, 0, 1, r.getrandbits(32)]))
    idx = r.randrange(2)
    put(obj, 4, 'I', idx)
    r.floats(obj, range(8, 0x48, 4))
    if r.random() < 0.5:   # (whole and half steps of the table: fistp's ties)
        put(obj, 0x48, 'f', r.randrange(-3000, 3000) * (math.pi / 2048) * r.choice([1, 1, 0.5, 1.5]))
    else:
        put(obj, 0x48, 'I', r.f())
    r.floats(obj, range(0x4c, 0x70, 4))
    regions[OBJ] = obj
    rides = bytearray(r.bytes(0x10))
    put(rides, 4 * idx, 'I', HOLDER)
    regions[0x7a4610] = rides
    regions[HOLDER] = put(bytearray(r.bytes(0x10)), 8, 'I', INNER)
    regions[INNER] = put(bytearray(r.bytes(0x10)), 4, 'I', NODE + 0x100)
    regions[NODE + 0x100] = put(bytearray(r.bytes(0x80)), 0x78, 'I', NODE)
    node = bytearray(r.bytes(0x60))
    r.floats(node, range(0x10, 0x50, 4))
    regions[NODE] = node
    return Call(0x468e50, [OBJ], regions)


if __name__ == '__main__':
    B.main(case)
