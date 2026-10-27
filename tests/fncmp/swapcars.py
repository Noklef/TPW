# A train's cars swapped and edited (src/game/coaster/train: Train_SwapCars 0x4424e0, Car_Edited 0x443f40, Car_SetLengthAt 0x444150)
# against the originals, with what they call (Car_SetChannelRates, the couplings, Flags_SetChanged, Train_PlaceCars with Animator_Update
# the cdecl recorder k 27, CarType_Lerp, Contacts_MarkLostPair, Track_PieceAt). The trains, rides and type are insertcar.py's (two cars
# or more); the coaster [0x790fec] a block of its own: +0x54 the first car (the cars chained on by +0x64, now and then cut short), +0x60
# one of the cars or another, flags any. Ops: Swap (two of the cars, now and then the same), Edited (what 0, 1, 4, 5 or any; the car
# flag 0x40, the type flag 0x4000 now and then; keepContacts or not), SetLength (a length about 0..1 or special, any of the pieces).
# usage: swapcars.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, fb, put  # noqa: E402
import insertcar  # noqa: E402

B = Bench([(0x4424e0, 0x4425bb), (0x443f40, 0x444035), (0x444150, 0x44418e), (0x432d50, 0x432df0), (0x434650, 0x434740), (0x43e520, 0x43e540),
           (0x5da3c0, 0x5da3c1), (0x432df0, 0x433220), (0x433220, 0x433240), (0x430ca0, 0x430ee0), (0x469940, 0x46a560), (0x46f150, 0x46f190),
           (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)], hooks={0x473c70: _recorder(27, 3, cdecl=True)})
COASTER = RAM + 0x4000
OPS = {'Swap': 0x4424e0, 'Edited': 0x443f40, 'SetLength': 0x444150}


def case(r):
    op = r.deal(PICK)
    while True:   # (insertcar.py's world, with two cars or more)
        regions = insertcar.case(r).regions
        n = struct.unpack_from('<I', regions[insertcar.TRAIN], 0x30)[0]
        if n >= 2:
            break
    cars = [insertcar.CARS + insertcar.CAR * k for k in range(n)]
    carbuf = regions[insertcar.CARS]
    for k in range(n):   # (the chain Track_PieceAt walks)
        put(carbuf, insertcar.CAR * k + 0x64, 'I', cars[k + 1] if k + 1 < n and r.random() < 0.97 else 0)
    coaster = bytearray(r.bytes(0x120))
    put(coaster, 0x54, 'I', cars[0])
    put(coaster, 0x60, 'I', r.choice(cars) if r.random() < 0.7 else insertcar.CARS + insertcar.CAR * 5)
    regions[COASTER] = coaster
    regions[0x790fec] = COASTER.to_bytes(4, 'little')
    if op == 'Swap':
        a = r.randrange(n)
        b = a if r.random() < 0.05 else r.choice([k for k in range(n) if k != a])
        return Call(0x4424e0, [cars[a], cars[b]], regions)
    tp = regions[insertcar.TYPE]
    put(tp, 0, 'I', struct.unpack_from('<I', tp, 0)[0] & ~0x4000 | (0x4000 if r.random() < 0.3 else 0))
    if op == 'Edited':
        car = r.choice(cars) if r.random() < 0.3 else struct.unpack_from('<I', coaster, 0x60)[0]
        o = car - insertcar.CARS
        put(carbuf, o + 8, 'I', struct.unpack_from('<I', carbuf, o + 8)[0] & ~0x40 | r.choice([0, 0x40]))
        what = r.choice([0, 1, 1, 4, 4, 5, r.getrandbits(32)])
        return Call(0x443f40, [what, car, COASTER, r.getrandbits(32), r.choice([0, 1])], regions)
    length = r.choice([fb(r.uniform(-0.5, 1.5)), fb(r.random()), fb(r.choice([0.0, 1.0])), r.f()])
    return Call(0x444150, [length, r.randrange(n), r.getrandbits(32), r.choice([0, 1])], regions)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
