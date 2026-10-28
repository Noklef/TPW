# A car put into a train (src/game/coaster/train: Train_InsertCar 0x433bd0) against the original, with what it calls: Car_TrainLength's
# inlined loops, Car_SetChannelRates, Car_FitHeight, Train_ShrinkToTop, the inlined couplings and Train_PlaceCars (its Animator_Update
# the cdecl recorder k 27, as placecars.py; Model_FitToGrid with an owner without a grid). Trains of 0-5 cars (lengths about 0..1, flags
# 0x2000 now and then, slots beyond the count empty or another car) on three rides (each with its model, root, frames and tops, and its
# +0x10 block of five rate channels); the type's +0x68 / +0x6c, shortest / longest / gap and rate ranges as coaster.py's pylon and rates
# deal them; the car put in any slot up to the count (now and then the slot an empty one), height about the train, out or none, and
# mark, noInsert, noShrink and free.
# usage: insertcar.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put  # noqa: E402

B = Bench([(0x433bd0, 0x433f64), (0x432df0, 0x433220), (0x433220, 0x433240), (0x433280, 0x433300), (0x430ca0, 0x430ee0),
           (0x433790, 0x4338e0), (0x433a90, 0x433bd0), (0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0),
           (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)], hooks={0x473c70: _recorder(27, 3, cdecl=True)})
TYPE, TRAIN, CARS, PIECES, RIDES, MODELS, ROOTS = RAM, RAM + 0x200, RAM + 0x300, RAM + 0xa00, RAM + 0xa80, RAM + 0xb00, RAM + 0xc00
FRAMES, OTHERS, FLAGS, WORLD, OWNER, OUT = RAM + 0x1000, RAM + 0x1600, RAM + 0x1c80, RAM + 0x1d00, RAM + 0x1d80, RAM + 0x1da0
BLOCKS, TOPS = RAM + 0x2000, RAM + 0x3000
CAR, NR, NF, NO = 0x110, 3, 3, 6


def case(r):
    log = log_region()
    tp = bytearray(r.bytes(0x130))
    put(tp, 0, 'I', r.getrandbits(32) & ~0x3c18 | (r.getrandbits(5) << 10 & 0x3c00) | r.choice([0, 0x10]) | r.choice([0, 8]))
    put(tp, 0x18, 'I', r.randrange(3))
    for at in (0x1c, 0x20, 0x24, 0x28):
        put(tp, at, 'I', r.randrange(4))
    shortest = r.uniform(0.5, 4)
    put(tp, 0xbc, 'fff', shortest, shortest + r.choice([r.uniform(0.5, 4)] * 5 + [0.0]), r.uniform(0, 1))
    low = r.uniform(0, 6)
    put(tp, 0x68, 'ff', low, low + r.choice([r.uniform(0, 30), r.uniform(0, 30), r.uniform(0, 5), -1.0]))
    if r.random() < 0.05:
        put(tp, r.choice([0x68, 0x6c, 0xbc, 0xc4]), 'I', r.choice([0x7fc00000, 0xffc00000]))
    for k in range(5):   # the rate ranges (coaster.py's rate_env)
        b = 0xd0 + 0x10 * k
        lo = r.uniform(-3, 1)
        put(tp, b, 'ff', lo, lo + r.choice([r.uniform(1, 6), r.uniform(1, 6), r.uniform(0, 1), 0.0, -1.0]))
        put(tp, b + 8, 'II', r.choice([fb(r.uniform(-2, 2)), r.f()]), r.choice([fb(r.uniform(-1, 1)), r.f()]))
    n = r.choice([0, 1, 2, 3, 3, 4, 5])
    train = bytearray(r.bytes(0x60))
    train[4] = train[4] & ~0x21 | r.choice([0, 0, 0, 1]) | r.choice([0, 0x20])
    put(train, 0x18, 'I', TYPE)
    put(train, 0x20, 'I', r.f(20))
    put(train, 0x30, 'I', n)
    cars = bytearray(r.bytes(CAR * 6))
    for k in range(6):
        o = CAR * k
        put(cars, o + 4, 'I', k)
        put(cars, o + 8, 'I', r.getrandbits(32) & ~0x12002100 | (0x2000 if r.random() < 0.2 else 0) | r.choice([0, 0x10000000]) |
            r.choice([0, 0x100]))
        put(cars, o + 0x50, 'I', r.f(20))
        put(cars, o + 0x68, 'I', PIECES + 0x10 * k if r.random() < 0.7 else 0)
        put(cars, o + 0x70, 'I', r.randrange(NR))
        put(cars, o + 0x100, 'I', TRAIN)
        for v in range(0xe8, 0xfc, 4):
            put(cars, o + v, 'I', r.choice([fb(r.uniform(-0.5, 1.5)), fb(r.uniform(-3, 3)), r.f()]))
        put(cars, o + 0xe8, 'I', r.choice([fb(r.random()), fb(r.random()), fb(r.choice([0.0, 1.0])), r.f()]))
    for k in range(6):   # (the last car, index 5, is the one put in when the train has five)
        put(train, 0x34 + 4 * k, 'I', CARS + CAR * k if k < n else r.choice([0, CARS + CAR * k]))
    slot = r.randrange(n + 1)
    car = CARS + CAR * (5 if n == 5 else r.randrange(n, 6))
    no_insert = r.random() < 0.2
    if r.random() < 0.05 and slot < n and no_insert:
        put(train, 0x34 + 4 * slot, 'I', 0)   # (an empty slot: none to fit; an inserted car would move it)
    rides, models, roots, tops = bytearray(r.bytes(0x80)), bytearray(r.bytes(0x100)), bytearray(r.bytes(0xa0 * NR)), bytearray(r.bytes(0x600))
    frames, others, flags = bytearray(r.bytes(NR * NF * 0xa0)), bytearray(r.bytes(NR * NO * 0x58)), bytearray(r.bytes(0x40))
    blocks = bytearray(r.bytes(NR * 0x200))
    for k in range(NR):
        put(rides, 4 * k, 'I', RIDES + 0x20 + 0x20 * k)   # the table at 0x7a4610; a ride's +8 its model, +0x10 its rate block
        put(rides, 0x20 + 0x20 * k + 8, 'I', MODELS + 0x50 * k)
        put(rides, 0x20 + 0x20 * k + 0x10, 'I', BLOCKS + 0x200 * k)
        r.floats(blocks, [0x200 * k + 0x38 * c + 0x1c for c in range(5)])
        m = 0x50 * k
        put(models, m + 4, 'I', ROOTS + 0xa0 * k)
        put(models, m + 0x28, 'I', TOPS + 0x200 * k)
        put(models, m + 0x2c, 'I', FLAGS + 4 * k if r.random() < 0.8 else 0)
        t = 0x200 * k
        put(tops, t + 4, 'I', TOPS + t + 0x10)
        for row in range(3):
            put(tops, t + 0x10 + 20 * row + 4, 'I', TOPS + t + 0x60 + 0x40 * row if r.random() < 0.8 else 0)
            put(tops, t + 0x60 + 0x40 * row + 0x34, 'I', r.f(5))
        o = 0xa0 * k
        put(roots, o + 0x44, 'H', r.randrange(1, NF + 1))
        put(roots, o + 0x46, 'H', r.choice([0, 0, 1, 2]))
        put(roots, o + 0x70, 'I', FRAMES + NF * 0xa0 * k)
        put(roots, o + 0x74, 'I', OTHERS + NO * 0x58 * k)
        put(roots, o + 0x78, 'I', FRAMES + NF * 0xa0 * k)
        put(roots, o + 0x9c, 'I', ROOTS + o)
    world = bytearray(r.bytes(0x80))
    put(world, 0x6c, 'I', 0)
    regions = {LOG: log, TYPE: tp, TRAIN: train, CARS: cars, PIECES: r.bytes(0x60), RIDES: rides, MODELS: models, ROOTS: roots,
               TOPS: tops, FRAMES: frames, OTHERS: others, FLAGS: flags, WORLD: world, BLOCKS: blocks, OWNER: r.bytes(0x10), OUT: r.bytes(4),
               0x7a4610: rides[:4 * NR], 0x790afc: bytes(4) if r.random() < 0.85 else r.bytes(4),
               0x7a0854: WORLD.to_bytes(4, 'little'), 0x790fec: OWNER.to_bytes(4, 'little')}
    height = r.choice([fb(r.uniform(-5, 40)), fb(r.uniform(-5, 40)), fb(r.uniform(0, 10)), r.f()])
    args = [OWNER, car, TRAIN, slot, height, OUT if r.random() < 0.7 else 0, r.choice([0, 1]), int(no_insert), r.choice([0, 0, 1]),
            r.choice([0, 0, 0, 1])]
    return Call(0x433bd0, args, regions)


if __name__ == '__main__':
    B.main(case)
