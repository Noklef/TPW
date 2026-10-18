# Riders boarding a coaster's cars (src/game/coaster/cars: 0x43b600) against the original: a queue ring of 1 to 6 slots (riders 0, the
# things 1-3 of partstate.py's fixture or other handles; the count 0 to all, the read position anywhere in it), a coaster with 0 to 5
# riders to take and 1 to 6 cars of 0x60 bytes (seats free or full, seat counts random), first and count over them (count 0 too), the
# rand() seed random; each seating activates a part through PartStates_Activate on partstate.py's fixture (the car kinds' entries in
# 0x7a4610 all hold its instance).
# Station (0x43b8b0): the same coaster's train at the station, arriving (riders getting off: CoasterTrain_Unload, PartStates_Release on the
# fixture), staying or leaving: train front / back about the coaster's segment window (now and then NaN), flags 0x20 and 4 set or not,
# a car always in the window (CoasterCar_FindSegmentsInWindow), the last window (+0x18 / +0x1c) and rider lists random but in range, the
# coaster's boarding ring (+0xd0) with room or full.
# usage: boarding.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=Board,Station)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import HEAP, Bench, Call, RAM, heap, put  # noqa: E402
import partstate  # noqa: E402

B = Bench(partstate.RANGES + [(0x43b600, 0x43b745), (0x43b8b0, 0x43ba1a), (0x43b480, 0x43b600), (0x43b7e0, 0x43b8b0), (0x44b4c0, 0x44b505),
                              (0x475550, 0x47558b)])
QUEUE, RING, COASTER, CARS, SEATS, PARTS, KINDS = (RAM + o for o in (0x20000, 0x20100, 0x20200, 0x21000, 0x22000, 0x23000, 0x24000))
KIND_TABLE, SEED = 0x7a4610, 0x7b0958
BRING, OFF = RAM + 0x25000, RAM + 0x26000   # (Station's: the coaster's boarding ring, the riders getting off)


def case(r):
    return station(r) if r.deal(OPS) == 'Station' else board(r)


def f32(v):
    return struct.unpack('<f', struct.pack('<f', v))[0]


def station(r):
    c = board(r)
    regions = dict(c.regions)
    m = len(regions[CARS]) // 0x60
    coaster = bytearray(regions[QUEUE]) + bytearray(r.bytes(0x30))
    put(coaster, 8, 'I', r.getrandbits(32) | 4 if r.random() < 0.7 else r.getrandbits(32) & ~4)
    cap = r.choice([0, 1, 2, 4, 8])   # the boarding ring: room for cap, count and pending below or at it
    put(coaster, 0xd0, 'II', cap, BRING)
    put(coaster, 0xdc, 'II', r.randrange(cap + 1), r.choice([0, 0, 1]))
    put(coaster, 0xe8, 'I', BRING + 4 * r.randrange(max(cap, 1)))
    lo = f32(r.uniform(0, 15))
    hi = f32(lo + r.uniform(2, 10))
    put(coaster, 0xac, 'f', r.choice([1.0, 1.0, 2.0, 0.5]))
    put(coaster, 0xf8, 'ff', lo, hi)
    start = r.randrange(m)
    count = r.randrange(1, m - start + 1)
    last = r.randrange(m)
    was = r.randrange(min(count, last) + 1)
    train = bytearray(regions[COASTER])
    put(train, 0, 'I', r.getrandbits(32) | 0x20 if r.random() < 0.5 else r.getrandbits(32) & ~0x20)
    put(train, 8, 'II', start, count)
    put(train, 0x10, 'I', r.randrange(20))
    put(train, 0x18, 'II', was, last if last >= was else was)
    edge = r.choice([lo, hi, lo - 1, hi + 1, r.uniform(-5, 30)])
    put(train, 0x20, 'ff', *r.choice([(edge, edge - 2), (hi, lo), (lo - 3, lo - 5), (hi + 5, hi + 3), (lo + 1, edge)]))
    if r.random() < 0.05:
        put(train, r.choice([0x20, 0x24]), 'f', float('nan'))
    cars, parts = bytearray(regions[CARS]), bytearray(regions[PARTS])
    off = bytearray(0x20 * m)
    inside = r.randrange(start, start + count)
    for k in range(m):
        put(cars, 0x60 * k + 0x28, 'I', OFF + 0x20 * k)
        put(cars, 0x60 * k + 0x30, 'I', r.choice([0, 0, 1, 2, 4]))
        put(off, 0x20 * k, '4I', *[r.choice([1, 2, 3, 0, r.getrandbits(32)]) for _ in range(4)])
        put(cars, 0x60 * k + 0x40, 'f', (lo + hi) / 2 if k == inside else r.uniform(-5, 30))
        put(parts, 0x40 * k + 0x24, 'ff', *((0.5, 0.5) if k == inside else (r.uniform(0, 2), r.uniform(0, 2))))
    regions.update({QUEUE: bytes(coaster), COASTER: bytes(train), CARS: bytes(cars), PARTS: bytes(parts), OFF: bytes(off),
                    BRING: r.bytes(0x20)})
    return Call(0x43b8b0, [QUEUE, COASTER], regions, ret='v')


def board(r):
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


OPS = [k for k in os.environ.get('OPS', 'Board,Station').split(',') if k]

if __name__ == '__main__':
    B.main(case)
