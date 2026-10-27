# A train's cars swapped and edited (src/game/coaster/train: Train_SwapCars 0x4424e0, Car_Edited 0x443f40, Car_SetLengthAt 0x444150,
# Car_ApplyLengthAt 0x444190, Car_SetE0At 0x4441d0, Car_ApplyE0At 0x444220, Car_SetAnglesAt 0x444270, Car_ApplyAnglesAt 0x4442f0,
# Car_SwapWithNextAt 0x447ae0, Car_SwapWithPrevAt 0x447b20)
# against the originals, with what they call (Car_SetChannelRates, the couplings, Flags_SetChanged, Train_PlaceCars with Animator_Update
# the cdecl recorder k 27, CarType_Lerp, Contacts_MarkLostPair, Track_PieceAt). The trains, rides and type are insertcar.py's (two cars
# or more); the coaster [0x790fec] a block of its own: +0x54 the first car (the cars chained on by +0x64, now and then cut short), +0x60
# one of the cars or another, flags any. Ops: Swap (two of the cars, now and then the same), Edited (what 0, 1, 4, 5 or any; the car
# flag 0x40, the type flag 0x4000 now and then, its length a NaN 15%; keepContacts or not), SetLength (a length about 0..1, special or
# a NaN, any of the pieces; ApplyLength the same), SetE0 / ApplyE0 / SetAngles / ApplyAngles (values any, the piece's flags 0x40 and
# 0x1 / 0x6 now and then, half the time the coaster's +0x60 car; mark 0, 1 or any; keep 0 or 1), SwapNext / SwapPrev (a piece with a
# car after / before it, 5% the last / first: past the slots, or the count, sic; the index's high bits any). Swap: now and then a's length a NaN with b's rates skipped, or a's ride's value a NaN going to b's ride,
# which no placed car rewrites (b alone on it, flagged 0x2000): what the x87 makes of them reaches b.
# usage: swapcars.py [CASES [SEED]] [--wild|--both] [--show N]   (env OPS=name,name to pick)
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', 'tools'))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from fncmp import RAM, Bench, Call, _recorder, fb, put  # noqa: E402
import insertcar  # noqa: E402

B = Bench([(0x4424e0, 0x4425bb), (0x443f40, 0x444035), (0x444150, 0x44418e), (0x444190, 0x4441c8), (0x4441d0, 0x444217), (0x444220, 0x444261), (0x444270, 0x4442ee), (0x4442f0, 0x444360), (0x447ae0, 0x447b14), (0x447b20, 0x447b54),
           (0x432d50, 0x432df0), (0x434650, 0x434740), (0x43e520, 0x43e540),
           (0x5da3c0, 0x5da3c1), (0x432df0, 0x433220), (0x433220, 0x433240), (0x430ca0, 0x430ee0), (0x469940, 0x46a560), (0x46f150, 0x46f190),
           (0x46fc00, 0x46fcb0), (0x46fd40, 0x470100), (0x4702d0, 0x4704d0)], hooks={0x473c70: _recorder(27, 3, cdecl=True)})
COASTER = RAM + 0x4000
OPS = {'Swap': 0x4424e0, 'Edited': 0x443f40, 'SetLength': 0x444150, 'ApplyLength': 0x444190, 'SetE0': 0x4441d0, 'ApplyE0': 0x444220,
       'SetAngles': 0x444270, 'ApplyAngles': 0x4442f0, 'SwapNext': 0x447ae0, 'SwapPrev': 0x447b20}


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
    if op in ('SwapNext', 'SwapPrev'):   # (a car's train slot +4 is its piece index here: insertcar.py puts car k in slot k)
        nxt = op == 'SwapNext'
        edge = r.random() < 0.05
        index = (n - 1 if nxt else 0) if edge else (r.randrange(n - 1) if nxt else r.randrange(1, n))
        return Call(OPS[op], [index | r.choice([0, r.getrandbits(16) << 16])], regions)
    if op == 'Swap':
        a = r.randrange(n)
        b = a if r.random() < 0.05 else r.choice([k for k in range(n) if k != a])
        nan = r.choice([0x7f800001, 0xff812345, 0x7fc00000])
        if r.random() < 0.25:   # (a's length a NaN, kept while b's rates are skipped: the train's flag 1 or b's 0x2000)
            put(carbuf, insertcar.CAR * a + 0xe8, 'I', nan)
            train = regions[insertcar.TRAIN]
            train[4] |= r.choice([0, 1])
            o = insertcar.CAR * b + 8
            put(carbuf, o, 'I', struct.unpack_from('<I', carbuf, o)[0] | r.choice([0, 0x2000]))
        if r.random() < 0.25 and a != b:   # (a's ride's value a NaN, to b's ride, which b alone has, flagged 0x2000: no placed car rewrites it)
            ride = struct.unpack_from('<I', carbuf, insertcar.CAR * a + 0x70)[0]
            put(regions[insertcar.FRAMES], insertcar.NF * 0xa0 * ride + 0x44, 'I', nan)
            for k in range(n):
                put(carbuf, insertcar.CAR * k + 0x70, 'I', (ride + 1) % insertcar.NR if k == b else ride)
            o = insertcar.CAR * b + 8
            put(carbuf, o, 'I', struct.unpack_from('<I', carbuf, o)[0] | 0x2000)
        return Call(0x4424e0, [cars[a], cars[b]], regions)
    tp = regions[insertcar.TYPE]
    put(tp, 0, 'I', struct.unpack_from('<I', tp, 0)[0] & ~0x4000 | (0x4000 if r.random() < 0.3 else 0))
    if op == 'Edited':
        car = r.choice(cars) if r.random() < 0.3 else struct.unpack_from('<I', coaster, 0x60)[0]
        o = car - insertcar.CARS
        put(carbuf, o + 8, 'I', struct.unpack_from('<I', carbuf, o + 8)[0] & ~0x40 | r.choice([0, 0x40]))
        if r.random() < 0.15:
            put(carbuf, o + 0xe8, 'I', r.choice([0x7f800001, 0xff812345, 0x7fc00000, 0xffc00000]))
        what = r.choice([0, 1, 1, 4, 4, 5, r.getrandbits(32)])
        return Call(0x443f40, [what, car, COASTER, r.getrandbits(32), r.choice([0, 1])], regions)
    length = r.choice([fb(r.uniform(-0.5, 1.5)), fb(r.random()), fb(r.choice([0.0, 1.0])), r.f(), r.choice([0x7f800001, 0xffc00000])])
    if op == 'SetLength':
        return Call(0x444150, [length, r.randrange(n), r.getrandbits(32), r.choice([0, 1])], regions)
    index = r.randrange(n)
    if op == 'ApplyLength':
        return Call(0x444190, [length, index], regions)
    o = insertcar.CAR * index
    put(carbuf, o + 8, 'I', struct.unpack_from('<I', carbuf, o + 8)[0] & ~0x47 | r.choice([0, 0x40]) | r.choice([0, 1, 6, 7]))
    if r.random() < 0.5:
        put(coaster, 0x60, 'I', cars[index])
    a, b = r.f(), r.f()
    mark = r.choice([0, 1, r.getrandbits(32)])
    keep = r.choice([0, 1])
    if op == 'SetE0':
        return Call(0x4441d0, [a, index, mark, keep], regions)
    if op == 'ApplyE0':
        return Call(0x444220, [a, index], regions)
    if op == 'SetAngles':
        return Call(0x444270, [a, b, index, mark, keep], regions)
    return Call(0x4442f0, [a, b, index], regions)


PICK = [k for k in os.environ.get('OPS', ','.join(OPS)).split(',') if k]

if __name__ == '__main__':
    B.main(case)
