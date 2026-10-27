# A train's cars placed (src/game/coaster/train: Train_PlaceCars 0x432df0) against the original: trains of 1-5 cars (flag 0x2000 now and
# then, a piece at +0x68 or none) on three rides (table 0x7a4610), each ride's model with its root (frames 0xa0 and 0x58 apart, the
# root frame's +0x44 the base height), its flags dword (+0x2c, or none) and its tops table (+0x28 -> +4, 20-byte rows, a top's +0x34);
# the type's flags (0x10, 0x400, 0x800, 0x1000, 0x2000), row and frame indexes; 0x790afc and force; keep or not. Animator_Update
# (0x473c70) is the cdecl recorder k 27 (three arguments), as the port's TrainEnv in the driver; Model_FitToGrid runs (gridfit's
# ranges) with the owner [0x7a0854] whose grid +0x6c is null, so it returns after reading the root and its holder.
# usage: placecars.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
from fncmp import LOG, RAM, Bench, Call, _recorder, fb, log_region, put  # noqa: E402

B = Bench([(0x432df0, 0x433220), (0x433220, 0x433240), (0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0),
           (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)], hooks={0x473c70: _recorder(27, 3, cdecl=True)})
TYPE, TRAIN, CARS, PIECES = RAM, RAM + 0x100, RAM + 0x200, RAM + 0x800
RIDES, MODELS, ROOTS, TOPS = RAM + 0x900, RAM + 0x980, RAM + 0xa80, RAM + 0x3000
FRAMES, OTHERS, FLAGS, WORLD = RAM + 0x1000, RAM + 0x2000, RAM + 0x2c00, RAM + 0x2d00
CAR, NR, NF, NO = 0x110, 3, 3, 6


def case(r):
    log = log_region()
    tp = bytearray(r.bytes(0xd0))
    put(tp, 0, 'I', r.getrandbits(32) & ~0x3c10 | (r.getrandbits(5) << 10 & 0x3c00) | r.choice([0, 0x10]))
    put(tp, 0x18, 'I', r.randrange(3))
    for at in (0x1c, 0x20, 0x24, 0x28):
        put(tp, at, 'I', r.randrange(4))   # (plus the root's +0x46, 0-2: within the frames and the others)
    put(tp, 0xc4, 'I', r.f(5))
    n = r.choice([1, 2, 3, 3, 4, 5])
    train = bytearray(r.bytes(0x60))
    put(train, 0x18, 'I', TYPE)
    put(train, 0x20, 'I', r.f(20))
    put(train, 0x30, 'I', n)
    cars = bytearray(r.bytes(CAR * 5))
    pieces = bytearray(r.bytes(0x50))
    for k in range(5):
        put(train, 0x34 + 4 * k, 'I', CARS + CAR * k)
        o = CAR * k
        put(cars, o + 8, 'I', r.getrandbits(32) & ~0x2000 | (0x2000 if r.random() < 0.2 else 0))
        put(cars, o + 0x50, 'I', r.f(20))
        put(cars, o + 0x68, 'I', PIECES + 0x10 * k if r.random() < 0.7 else 0)
        put(cars, o + 0x70, 'I', r.randrange(NR))
        put(cars, o + 0x100, 'I', CARS + o + 0x80)   # its part, +0x20 the lift
        put(cars, o + 0x80 + 0x20, 'I', r.f(5))
    rides, models, roots, tops = bytearray(r.bytes(0x80)), bytearray(r.bytes(0x100)), bytearray(r.bytes(0xa0 * NR)), bytearray(r.bytes(0x600))
    frames, others, flags = bytearray(r.bytes(NR * NF * 0xa0)), bytearray(r.bytes(NR * NO * 0x58)), bytearray(r.bytes(0x40))
    for k in range(NR):
        put(rides, 4 * k, 'I', RIDES + 0x20 + 0x10 * k)   # the table at 0x7a4610
        put(rides, 0x20 + 0x10 * k + 8, 'I', MODELS + 0x50 * k)
        m = 0x50 * k
        put(models, m + 4, 'I', ROOTS + 0xa0 * k)
        put(models, m + 0x28, 'I', TOPS + 0x200 * k)
        put(models, m + 0x2c, 'I', FLAGS + 4 * k if r.random() < 0.8 else 0)
        t = 0x200 * k   # the tops: +4 the table (20-byte rows from +0x10), a row's +4 a top (0x40 apart from +0x60) or none
        put(tops, t + 4, 'I', TOPS + t + 0x10)
        for row in range(3):
            put(tops, t + 0x10 + 20 * row + 4, 'I', TOPS + t + 0x60 + 0x40 * row if r.random() < 0.8 else 0)
            put(tops, t + 0x60 + 0x40 * row + 0x34, 'I', r.f(5))
        o = 0xa0 * k
        count = r.randrange(1, NF + 1)
        put(roots, o + 0x44, 'H', count)
        put(roots, o + 0x46, 'H', r.choice([0, 0, 1, 2]))
        put(roots, o + 0x70, 'I', FRAMES + NF * 0xa0 * k)
        put(roots, o + 0x74, 'I', OTHERS + NO * 0x58 * k)
        put(roots, o + 0x78, 'I', FRAMES + NF * 0xa0 * k)
        put(roots, o + 0x9c, 'I', ROOTS + o)
    world = bytearray(r.bytes(0x80))
    put(world, 0x6c, 'I', 0)
    regions = {LOG: log, TYPE: tp, TRAIN: train, CARS: cars, PIECES: pieces, RIDES: rides, MODELS: models, ROOTS: roots, TOPS: tops,
               FRAMES: frames, OTHERS: others, FLAGS: flags, WORLD: world, 0x7a4610: rides[:4 * NR],
               0x790afc: bytes(4) if r.random() < 0.7 else r.bytes(4), 0x7a0854: WORLD.to_bytes(4, 'little')}
    return Call(0x432df0, [TYPE, TRAIN, r.choice([0, 1]), r.choice([0, 0, 1])], regions)


if __name__ == '__main__':
    B.main(case)
