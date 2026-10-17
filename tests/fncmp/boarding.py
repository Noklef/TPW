# Riders boarding a coaster's cars (src/game/coaster/cars: 0x43b600) against the original: a queue ring of 1 to 6 slots (riders 0, the
# things 1-3 of partstate.py's fixture or other handles; the count 0 to all, the read position anywhere in it), a coaster with 0 to 5
# riders to take and 1 to 6 cars of 0x60 bytes (seats free or full, seat counts random), first and count over them (count 0 too), the
# rand() seed random; each seating activates a part through PartStates_Activate on partstate.py's fixture (the car kinds' entries in
# 0x7a4610 all hold its instance).
# usage: boarding.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402
import partstate  # noqa: E402

B = Bench(partstate.RANGES + [(0x43b600, 0x43b745)])
QUEUE, RING, COASTER, CARS, SEATS, PARTS, KINDS = (RAM + o for o in (0x20000, 0x20100, 0x20200, 0x21000, 0x22000, 0x23000, 0x24000))
KIND_TABLE, SEED = 0x7a4610, 0x7b0958


def case(r):
    regions = {HEAP: heap()}
    partstate.activation(r, regions)
    # the queue's ring
    n = r.randrange(1, 7)
    ring = struct.pack('<%dI' % n, *[r.choice([1, 2, 3, 3, 0, 0x10002, r.getrandbits(32)]) for _ in range(n)])
    queue = bytearray(r.bytes(0xd0))
    put(queue, 0xb0, 'II', n, RING)
    put(queue, 0xbc, 'I', r.choice([0, 1, 2, n, n, r.randrange(n + 1)]))
    put(queue, 0xc4, 'I', RING + 4 * r.randrange(n))
    # the coaster and its cars: seat parts 0-3, kinds 0-1 (both pointing at the fixture's instance)
    m = r.randrange(1, 7)
    coaster = bytearray(r.bytes(0x40))
    put(coaster, 0x14, 'I', r.choice([0, 1, 2, 3, 5]))
    put(coaster, 0x28, 'I', CARS)
    cars = bytearray(r.bytes(0x60 * m))
    for k in range(m):
        cap = r.choice([0, 1, 2, 4])
        put(cars, 0x60 * k + 4, 'I', PARTS + 0x40 * k)
        put(cars, 0x60 * k + 0xc, 'I', r.randrange(2))
        put(cars, 0x60 * k + 0x2c, 'I', SEATS + 0x20 * k)
        put(cars, 0x60 * k + 0x34, 'I', r.choice([0, 0, 1, cap, cap + 1]) if cap else r.choice([0, 1]))
        put(cars, 0x60 * k + 0x44, 'I', cap)
    parts = bytearray(r.bytes(0x40 * m))
    for k in range(m):
        put(parts, 0x40 * k + 0x2c, 'I', PARTS + 0x40 * k + 0x10)
        put(parts, 0x40 * k + 0x10, '6I', *[r.randrange(4) for _ in range(6)])
    kinds = bytearray(r.bytes(0x20))
    put(kinds, 8, 'I', partstate.INSTANCE)
    regions.update({QUEUE: bytes(queue), RING: ring, COASTER: bytes(coaster), CARS: bytes(cars), SEATS: r.bytes(0x20 * m), PARTS: bytes(parts),
                    KINDS: bytes(kinds), KIND_TABLE: struct.pack('<2I', KINDS, KINDS), SEED: struct.pack('<I', r.getrandbits(32))})
    first = r.randrange(m)
    count = r.choice([m - first, m - first, 1, r.randrange(m - first + 1)])
    return Call(0x43b600, [QUEUE, COASTER, first, count], regions, ret='v')


if __name__ == '__main__':
    B.main(case)
