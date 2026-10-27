# A car's length from a node (src/game/coaster/train: Car_LengthFromNode 0x435340) against the original, with Vec3_Distance and
# Car_SetLengthAt (and what it calls: swapcars.py's SetLength world, the coaster [0x790fec] with a value at +0x118). The node (+0x1c
# a position, +0x2c the next node's, +0x30 set or not); the range a block of its own (+0 flag 0x4000 or not; +0xbc..+0xc0 a span,
# now and then empty, reversed or with a NaN) or the type, or none now and then (crash in both); the car any of the train's (its +0
# the piece index Car_SetLengthAt edits); the other none (the range's pointer bits, sic) or a car, whose train flag 1 (+4) gives its
# +0x50, else its ride's top entry (the type's +0x2c: 0-2 or none) +0x34; keep, wantValue 0 or 1; out or none.
# usage: lengthfromnode.py [CASES [SEED]] [--wild|--both] [--show N]
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, fb, put  # noqa: E402
import insertcar  # noqa: E402
import swapcars  # noqa: E402

B = Bench([(0x435340, 0x4354fe), (0x470940, 0x470990), (0x4424e0, 0x4425bb), (0x443f40, 0x444035), (0x444150, 0x44418e), (0x432d50, 0x432df0),
           (0x434650, 0x434740), (0x43e520, 0x43e540), (0x5da3c0, 0x5da3c1), (0x432df0, 0x433220), (0x433220, 0x433240), (0x430ca0, 0x430ee0),
           (0x469940, 0x46a560), (0x46f150, 0x46f190), (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)],
          hooks={0x473c70: _recorder(27, 3, cdecl=True)})
NODE, NODE2, RANGE, OUTV = RAM + 0x5000, RAM + 0x5040, RAM + 0x5080, RAM + 0x5180


def pos(r):
    return [fb(r.uniform(-10, 10)) if r.random() < 0.97 else r.f() for _ in range(3)]


def case(r):
    swapcars.PICK = ['SetLength']
    regions = swapcars.case(r).regions
    n = struct.unpack_from('<I', regions[insertcar.TRAIN], 0x30)[0]
    cars = [insertcar.CARS + insertcar.CAR * k for k in range(n)]
    carbuf = regions[insertcar.CARS]
    for k in range(n):   # (the index Car_SetLengthAt edits)
        put(carbuf, insertcar.CAR * k, 'I', r.randrange(n) if r.random() < 0.97 else n)
    tp = regions[insertcar.TYPE]
    put(tp, 0x2c, 'I', r.randrange(3))
    train = regions[insertcar.TRAIN]
    train[4] = train[4] & ~1 | r.choice([0, 1])
    coaster = regions[swapcars.COASTER]
    put(coaster, 0x118, 'I', r.choice([fb(r.random()), fb(r.uniform(-5, 20)), r.f()]))
    node = bytearray(r.bytes(0x40))
    put(node, 0x1c, '3I', *pos(r))
    put(node, 0x2c, 'I', NODE2)
    put(node, 0x30, 'I', r.choice([0, 0, r.getrandbits(32)]))
    node2 = put(bytearray(r.bytes(0x40)), 0x1c, '3I', *pos(r))
    rng = bytearray(r.bytes(0x100))
    put(rng, 0, 'I', r.getrandbits(32) & ~0x4000 | r.choice([0, 0x4000, 0x4000]))
    low = r.uniform(-5, 10)
    put(rng, 0xbc, 'ff', low, low + r.choice([r.uniform(0.5, 20)] * 4 + [0.0, -1.0]))
    if r.random() < 0.05:
        put(rng, r.choice([0xbc, 0xc0]), 'I', r.choice([0x7fc00000, 0xffc00000, 0x7f800001]))
    put(tp, 0, 'I', struct.unpack_from('<I', tp, 0)[0] & ~0x4000 | r.choice([0, 0x4000]))
    regions.update({NODE: node, NODE2: node2, RANGE: rng, OUTV: r.bytes(4)})
    rangep = r.choice([RANGE] * 6 + [insertcar.TYPE]) if r.random() < 0.97 else 0
    other = r.choice(cars + [0]) if r.random() < 0.9 else insertcar.CARS + insertcar.CAR * 5
    args = [NODE, rangep, r.choice(cars), other, r.choice([0, 0, 1]), r.choice([0, 1]), r.choice([OUTV, 0])]
    return Call(0x435340, args, regions, ret='d')


if __name__ == '__main__':
    B.main(case)
